// ESP32-S3 on the JUNKR ECU board (rev A): the logger and Wi-Fi side.
//
//   STM32 --2 Mbaud UART--> here --> microSD  /drives/drive_00012.jlog
//                                 \-> Wi-Fi   list, download, live view, auto-upload
//
// Every byte from the STM32 goes to the card unchanged (it is already
// framed and CRC'd jlog, shared/ecu_log.h), so the file is exactly what the
// engine controller said. A parser runs alongside only to know the engine
// state (for the live view and for when to start a new drive file).
//
// Wi-Fi: always an access point (JUNKR-ECU) so a phone can connect in the
// truck; also joins your home network when it can see it, and then uploads
// finished drives to upload_url if you set one. Settings: /junkr.json on the
// card, see docs/standalone-ecu/LOGGING.md.
//
// Build: pio run -e ecu-comms
#include <Arduino.h>
#include <ESPmDNS.h>
#include <FS.h>
#include <HTTPClient.h>
#include <SD.h>
#include <SPI.h>
#include <WebServer.h>
#include <WiFi.h>
#include <time.h>

#include "ecu_log.h"

#ifndef FW_VERSION
#define FW_VERSION "dev"
#endif

// ---- rev A pins (kicad/standalone_ecu/tools/circuit.py, sheet 9) ---------
constexpr int PIN_STM_TX = 15;   // ESP TX -> STM32 USART1 RX
constexpr int PIN_STM_RX = 16;   // ESP RX <- STM32 USART1 TX
constexpr int PIN_SD_CS = 10, PIN_SD_MOSI = 11, PIN_SD_SCK = 12, PIN_SD_MISO = 13;
constexpr int PIN_LED = 2;
constexpr uint32_t LINK_BAUD = 2000000;   // must match the STM32 (src/ecu/stm32/main.cpp)

// ---- policy -----------------------------------------------------------------
constexpr uint32_t FLUSH_MS = 2000;                 // power loss costs at most this much log
constexpr uint32_t ENGINE_OFF_SPLIT_MS = 10 * 60 * 1000;   // engine off this long: next start is a new drive
constexpr uint32_t LINK_TIMEOUT_MS = 500;
constexpr uint32_t MAX_FILE_BYTES = 512u << 20;
constexpr uint64_t MIN_FREE_BYTES = 256ull << 20;   // delete oldest uploaded drives below this

// ---- settings (/junkr.json) --------------------------------------------------
struct Settings {
    String ap_ssid = "JUNKR-ECU", ap_pass = "junkr-kzj95";
    String sta_ssid, sta_pass;
    String upload_url, upload_token;
    String hostname = "junkr";
} cfg;

// Tiny "key": "value" reader; the file is flat on purpose.
static String json_str(const String &doc, const char *key) {
    String k = String("\"") + key + "\"";
    int i = doc.indexOf(k);
    if (i < 0) return "";
    i = doc.indexOf(':', i + k.length());
    if (i < 0) return "";
    i = doc.indexOf('"', i);
    if (i < 0) return "";
    int j = i + 1;
    String out;
    while (j < (int)doc.length() && doc[j] != '"') {
        if (doc[j] == '\\' && j + 1 < (int)doc.length()) j++;
        out += doc[j++];
    }
    return out;
}

static void load_settings() {
    File f = SD.open("/junkr.json");
    if (!f) {
        // First boot on a blank card: write a template to fill in.
        File w = SD.open("/junkr.json", FILE_WRITE);
        if (w) {
            w.print("{\n  \"ap_ssid\": \"JUNKR-ECU\",\n  \"ap_pass\": \"junkr-kzj95\",\n"
                    "  \"wifi_ssid\": \"\",\n  \"wifi_pass\": \"\",\n"
                    "  \"upload_url\": \"\",\n  \"upload_token\": \"\",\n  \"hostname\": \"junkr\"\n}\n");
            w.close();
        }
        return;
    }
    String doc = f.readString();
    f.close();
    auto set = [&](String &dst, const char *key) {
        String v = json_str(doc, key);
        if (v.length()) dst = v;
    };
    set(cfg.ap_ssid, "ap_ssid");
    set(cfg.ap_pass, "ap_pass");
    set(cfg.sta_ssid, "wifi_ssid");
    set(cfg.sta_pass, "wifi_pass");
    set(cfg.upload_url, "upload_url");
    set(cfg.upload_token, "upload_token");
    set(cfg.hostname, "hostname");
    if (cfg.ap_pass.length() < 8) cfg.ap_pass = "junkr-kzj95";   // WPA2 minimum
}

// ---- ring buffer: UART task -> SD writer --------------------------------
// 512 KB in PSRAM covers ~45 s of log if the card stalls.
static uint8_t *ring;
static constexpr size_t RING = 512 * 1024;
static volatile size_t r_head, r_tail;
static volatile uint32_t ring_overflow;
static portMUX_TYPE ring_mux = portMUX_INITIALIZER_UNLOCKED;

static void ring_put(const uint8_t *p, size_t n) {
    portENTER_CRITICAL(&ring_mux);
    size_t free_b = (r_tail + RING - r_head - 1) % RING;
    if (n > free_b) { ring_overflow += n; portEXIT_CRITICAL(&ring_mux); return; }
    for (size_t i = 0; i < n; i++) ring[(r_head + i) % RING] = p[i];
    r_head = (r_head + n) % RING;
    portEXIT_CRITICAL(&ring_mux);
}
static size_t ring_get(uint8_t *p, size_t cap) {
    portENTER_CRITICAL(&ring_mux);
    size_t avail = (r_head + RING - r_tail) % RING, n = avail < cap ? avail : cap;
    for (size_t i = 0; i < n; i++) p[i] = ring[(r_tail + i) % RING];
    r_tail = (r_tail + n) % RING;
    portEXIT_CRITICAL(&ring_mux);
    return n;
}

// ---- live state (from the parser) ------------------------------------------
struct LiveChan { String name, unit; char type; float scale; };
static LiveChan live_ch[64];
static int live_n;
static uint8_t live_sample[jlog::MAX_PAYLOAD];
static size_t live_len;
static uint8_t schema_frame[jlog::MAX_PAYLOAD + 8], meta_frame[jlog::MAX_PAYLOAD + 8];
static size_t schema_frame_len, meta_frame_len;
static float live_rpm;
static uint32_t last_byte_ms, last_running_ms, frames_good, frames_bad, bytes_in;
static bool link_up;
static SemaphoreHandle_t live_lock;

static void on_frame(const jlog::Parser &p) {
    xSemaphoreTake(live_lock, portMAX_DELAY);
    if (p.type() == jlog::T_SCHEMA) {
        const uint8_t *d = p.payload();
        int n = d[1] < 64 ? d[1] : 64;
        size_t k = 2;
        for (int i = 0; i < n && k < p.len(); i++) {
            live_ch[i].type = (char)d[k++];
            live_ch[i].scale = jlog::getf(d + k); k += 4;
            live_ch[i].name = (const char *)d + k; k += live_ch[i].name.length() + 1;
            live_ch[i].unit = (const char *)d + k; k += live_ch[i].unit.length() + 1;
        }
        live_n = n;
        schema_frame_len = jlog::frame(schema_frame, sizeof schema_frame, jlog::T_SCHEMA, d, p.len());
    } else if (p.type() == jlog::T_META) {
        meta_frame_len = jlog::frame(meta_frame, sizeof meta_frame, jlog::T_META, p.payload(), p.len());
    } else if (p.type() == jlog::T_SAMPLE) {
        live_len = p.len();
        memcpy(live_sample, p.payload(), live_len);
        // rpm is channel 0 in today's table, but look it up by name anyway.
        size_t k = 8;
        for (int i = 0; i < live_n; i++) {
            if (live_ch[i].name == "rpm") {
                live_rpm = (int16_t)jlog::get16(live_sample + k) / live_ch[i].scale;
                break;
            }
            k += jlog::type_size(live_ch[i].type);
        }
        if (live_rpm > 0) last_running_ms = millis();
    }
    xSemaphoreGive(live_lock);
}

static void link_event(uint32_t up) {
    uint8_t p[13], f[24];
    size_t n = jlog::frame(f, sizeof f, jlog::T_EVENT, p, jlog::event_payload(p, 0, jlog::E_LINK, up, millis()));
    ring_put(f, n);
}

static void uart_task(void *) {
    static jlog::Parser parser;
    static uint8_t buf[4096];
    for (;;) {
        int n = Serial1.read(buf, sizeof buf);
        if (n > 0) {
            ring_put(buf, n);
            bytes_in += n;
            last_byte_ms = millis();
            if (!link_up) { link_up = true; link_event(1); }
            for (int i = 0; i < n; i++)
                if (parser.push(buf[i])) on_frame(parser);
            frames_good = parser.good();
            frames_bad = parser.bad();
        } else {
            if (link_up && millis() - last_byte_ms > LINK_TIMEOUT_MS) { link_up = false; link_event(0); }
            vTaskDelay(pdMS_TO_TICKS(2));
        }
    }
}

// ---- drive files ------------------------------------------------------------
static File cur;
static String cur_name;
static uint32_t cur_bytes, next_drive = 1;
static bool sd_ok;
static SemaphoreHandle_t sd_lock;

static String drive_name(uint32_t n) {
    char b[32];
    snprintf(b, sizeof b, "drive_%05lu.jlog", (unsigned long)n);
    return b;
}

static void scan_drives() {
    File dir = SD.open("/drives");
    if (!dir) { SD.mkdir("/drives"); return; }
    for (File f = dir.openNextFile(); f; f = dir.openNextFile()) {
        String n = f.name();
        if (n.startsWith("drive_")) {
            uint32_t k = n.substring(6, 11).toInt();
            if (k >= next_drive) next_drive = k + 1;
        }
    }
}

static void open_drive() {
    cur_name = drive_name(next_drive++);
    cur = SD.open("/drives/" + cur_name, FILE_WRITE);
    cur_bytes = 0;
    if (!cur) return;
    char j[200];
    time_t now = time(nullptr);
    int n = snprintf(j, sizeof j, "{\"drive\":\"%s\",\"esp_fw\":\"%s\",\"boot_ms\":%lu,\"unix\":%ld}",
                     cur_name.c_str(), FW_VERSION, (unsigned long)millis(), now > 1700000000 ? (long)now : 0L);
    uint8_t f[256];
    size_t fl = jlog::frame(f, sizeof f, jlog::T_FILE, (const uint8_t *)j, n);
    cur.write(f, fl);
    // Self-contained from the first byte: repeat the last schema and meta.
    xSemaphoreTake(live_lock, portMAX_DELAY);
    if (schema_frame_len) cur.write(schema_frame, schema_frame_len);
    if (meta_frame_len) cur.write(meta_frame, meta_frame_len);
    xSemaphoreGive(live_lock);
    cur_bytes = fl + schema_frame_len + meta_frame_len;
}

static void close_drive() {
    if (cur) { cur.flush(); cur.close(); }
    cur_name = "";
}

static bool is_uploaded(const String &name) {
    File f = SD.open("/uploaded.txt");
    if (!f) return false;
    bool found = false;
    while (f.available() && !found) found = f.readStringUntil('\n') == name;
    f.close();
    return found;
}

// Keep room on the card: delete the oldest drives that are already uploaded.
static void make_room() {
    while (SD.totalBytes() - SD.usedBytes() < MIN_FREE_BYTES) {
        File dir = SD.open("/drives");
        String oldest;
        for (File f = dir.openNextFile(); f; f = dir.openNextFile()) {
            String n = f.name();
            if (n != cur_name && is_uploaded(n) && (oldest == "" || n < oldest)) oldest = n;
        }
        if (oldest == "") return;   // nothing safe to delete: keep logging until full
        SD.remove("/drives/" + oldest);
    }
}

static void writer_task(void *) {
    static uint8_t chunk[16384];
    uint32_t last_flush = millis();
    bool engine_was_off = false;
    for (;;) {
        size_t n = ring_get(chunk, sizeof chunk);
        xSemaphoreTake(sd_lock, portMAX_DELAY);
        if (sd_ok) {
            // A long engine-off ends the drive; the next start opens a new one.
            bool off_long = millis() - last_running_ms > ENGINE_OFF_SPLIT_MS;
            if (off_long && cur) { close_drive(); engine_was_off = true; }
            if (!cur && (!engine_was_off || live_rpm > 0)) {
                make_room();
                open_drive();
                engine_was_off = false;
            }
            if (cur && n) {
                cur.write(chunk, n);
                cur_bytes += n;
                if (cur_bytes > MAX_FILE_BYTES) { close_drive(); }
            }
            if (cur && millis() - last_flush > FLUSH_MS) { cur.flush(); last_flush = millis(); }
        }
        xSemaphoreGive(sd_lock);
        if (n < sizeof chunk) vTaskDelay(pdMS_TO_TICKS(20));
    }
}

// ---- web --------------------------------------------------------------------
static WebServer web(80);

static const char INDEX_HTML[] PROGMEM = R"HTML(<!doctype html><html><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1"><title>JUNKR ECU logs</title>
<style>body{font:15px system-ui;margin:16px;max-width:720px}table{border-collapse:collapse;width:100%}
td,th{padding:6px;border-bottom:1px solid #ddd;text-align:left}#live{font:13px ui-monospace,monospace;white-space:pre-wrap}
button{padding:4px 10px}</style></head><body><h2>JUNKR ECU</h2><p id="st"></p>
<h3>Drives</h3><table id="d"><tr><th>File</th><th>Size</th><th>Uploaded</th><th></th></tr></table>
<h3>Live</h3><div id="live"></div><script>
async function drives(){const r=await (await fetch('/api/drives')).json();
document.getElementById('st').textContent=`Recording ${r.recording||'(idle)'} · card free ${(r.free/1e9).toFixed(1)} GB · link ${r.link?'up':'down'}`;
const t=document.getElementById('d');t.querySelectorAll('tr.r').forEach(e=>e.remove());
for(const d of r.drives.reverse()){const tr=document.createElement('tr');tr.className='r';
tr.innerHTML=`<td><a href="/drives/${d.name}" download>${d.name}</a></td><td>${(d.size/1e6).toFixed(1)} MB</td><td>${d.uploaded?'yes':''}</td><td>${d.name==r.recording?'recording':''}</td>`;t.appendChild(tr);}}
async function live(){try{const r=await (await fetch('/api/live')).json();
document.getElementById('live').textContent=Object.entries(r).map(([k,v])=>k.padEnd(16)+v).join('\n');}catch(e){}}
drives();setInterval(drives,10000);setInterval(live,250);</script></body></html>)HTML";

static void api_drives() {
    String out = "{\"drives\":[";
    xSemaphoreTake(sd_lock, portMAX_DELAY);
    File dir = SD.open("/drives");
    bool first = true;
    for (File f = dir ? dir.openNextFile() : File(); f; f = dir.openNextFile()) {
        String n = f.name();
        if (!n.endsWith(".jlog")) continue;
        size_t size = (n == cur_name) ? cur_bytes : f.size();
        out += (first ? "" : ",");
        out += "{\"name\":\"" + n + "\",\"size\":" + String((unsigned long)size) +
               ",\"uploaded\":" + (is_uploaded(n) ? "true" : "false") + "}";
        first = false;
    }
    uint64_t free_b = sd_ok ? SD.totalBytes() - SD.usedBytes() : 0;
    xSemaphoreGive(sd_lock);
    out += "],\"recording\":\"" + cur_name + "\",\"free\":" + String((double)free_b, 0) +
           ",\"link\":" + (link_up ? "true" : "false") + ",\"frames_good\":" + frames_good +
           ",\"frames_bad\":" + frames_bad + ",\"ring_overflow\":" + ring_overflow + "}";
    web.send(200, "application/json", out);
}

static void api_live() {
    String out = "{";
    xSemaphoreTake(live_lock, portMAX_DELAY);
    size_t k = 8;
    for (int i = 0; i < live_n && k < live_len; i++) {
        const LiveChan &c = live_ch[i];
        double v;
        switch (c.type) {
            case 'h': v = (int16_t)jlog::get16(live_sample + k); break;
            case 'H': v = jlog::get16(live_sample + k); break;
            case 'i': v = (int32_t)jlog::get32(live_sample + k); break;
            default: v = jlog::get32(live_sample + k); break;
        }
        k += jlog::type_size(c.type);
        out += (i ? ",\"" : "\"") + c.name + "\":" + String(v / c.scale, c.scale >= 100 ? 2 : c.scale >= 10 ? 1 : 0);
    }
    xSemaphoreGive(live_lock);
    out += "}";
    web.send(200, "application/json", out);
}

static void serve_drive() {
    String uri = web.uri();
    String name = uri.substring(uri.lastIndexOf('/') + 1);
    if (name.indexOf("..") >= 0 || !name.endsWith(".jlog")) { web.send(400, "text/plain", "bad name"); return; }
    xSemaphoreTake(sd_lock, portMAX_DELAY);
    if (web.method() == HTTP_DELETE) {
        bool ok = name != cur_name && SD.remove("/drives/" + name);
        xSemaphoreGive(sd_lock);
        web.send(ok ? 200 : 409, "text/plain", ok ? "deleted" : "can't delete the drive being recorded");
        return;
    }
    if (name == cur_name && cur) cur.flush();
    File f = SD.open("/drives/" + name);
    xSemaphoreGive(sd_lock);
    if (!f) { web.send(404, "text/plain", "no such drive"); return; }
    // The SD lock isn't held while streaming; the writer only appends.
    web.sendHeader("Content-Disposition", "attachment; filename=" + name);
    web.streamFile(f, "application/octet-stream");
    f.close();
}

static void api_mode() {
    // Same single-byte commands as before; the STM32 decides whether
    // takeover is allowed (agreement run, RUN switch, supervisor).
    String m = web.arg("m");
    if (m == "S" || m == "T") { Serial1.write((uint8_t)m[0]); web.send(200, "text/plain", "sent"); }
    else web.send(400, "text/plain", "m=S (shadow) or m=T (request takeover)");
}

// ---- upload finished drives over home Wi-Fi --------------------------------
static void upload_task(void *) {
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(30000));
        if (!cfg.upload_url.length() || WiFi.status() != WL_CONNECTED) continue;
        String todo;
        xSemaphoreTake(sd_lock, portMAX_DELAY);
        File dir = SD.open("/drives");
        for (File f = dir ? dir.openNextFile() : File(); f; f = dir.openNextFile()) {
            String n = f.name();
            if (n.endsWith(".jlog") && n != cur_name && !is_uploaded(n) && (todo == "" || n < todo)) todo = n;
        }
        xSemaphoreGive(sd_lock);
        if (todo == "") continue;
        File f = SD.open("/drives/" + todo);
        if (!f) continue;
        HTTPClient http;
        http.begin(cfg.upload_url);
        http.addHeader("Content-Type", "application/octet-stream");
        http.addHeader("X-Junkr-Drive", todo);
        if (cfg.upload_token.length()) http.addHeader("Authorization", "Bearer " + cfg.upload_token);
        int code = http.sendRequest("POST", &f, f.size());
        http.end();
        f.close();
        if (code >= 200 && code < 300) {
            xSemaphoreTake(sd_lock, portMAX_DELAY);
            File u = SD.open("/uploaded.txt", FILE_APPEND);
            if (u) { u.println(todo); u.close(); }
            xSemaphoreGive(sd_lock);
        }
    }
}

// ---- setup / loop -------------------------------------------------------------
void setup() {
    Serial.begin(115200);
    pinMode(PIN_LED, OUTPUT);
    live_lock = xSemaphoreCreateMutex();
    sd_lock = xSemaphoreCreateMutex();
    ring = (uint8_t *)ps_malloc(RING);
    if (!ring) ring = (uint8_t *)malloc(RING / 8);   // no PSRAM: smaller buffer, still works

    SPI.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
    sd_ok = SD.begin(PIN_SD_CS, SPI, 20000000);
    if (sd_ok) { load_settings(); scan_drives(); }
    Serial.printf("JUNKR ECU comms %s, SD %s\n", FW_VERSION, sd_ok ? "ok" : "missing");

    Serial1.setRxBufferSize(32768);
    Serial1.begin(LINK_BAUD, SERIAL_8N1, PIN_STM_RX, PIN_STM_TX);

    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(cfg.ap_ssid.c_str(), cfg.ap_pass.c_str());
    if (cfg.sta_ssid.length()) WiFi.begin(cfg.sta_ssid.c_str(), cfg.sta_pass.c_str());
    WiFi.setAutoReconnect(true);
    configTime(0, 0, "pool.ntp.org", "time.google.com");
    MDNS.begin(cfg.hostname.c_str());
    MDNS.addService("http", "tcp", 80);

    web.on("/", [] { web.send_P(200, "text/html", INDEX_HTML); });
    web.on("/api/drives", api_drives);
    web.on("/api/live", api_live);
    web.on("/api/mode", HTTP_POST, api_mode);
    web.onNotFound([] {
        if (web.uri().startsWith("/drives/")) serve_drive();
        else web.send(404, "text/plain", "not found");
    });
    web.begin();

    xTaskCreatePinnedToCore(uart_task, "uart", 8192, nullptr, 5, nullptr, 1);
    xTaskCreatePinnedToCore(writer_task, "sd", 8192, nullptr, 4, nullptr, 1);
    xTaskCreatePinnedToCore(upload_task, "upload", 8192, nullptr, 1, nullptr, 0);
}

void loop() {
    web.handleClient();
    digitalWrite(PIN_LED, link_up ? (millis() / 100) & 1 : (millis() / 1000) & 1);   // fast blink = logging
    delay(2);
}
