// Dash Master Firmware (ESP32-S3 + Qualia) – v0.1
// Reads isolated SPI frames from ECU-Intercept board (ADS131M08)
// Publishes data to TFT gauge and forwards to Wi-Fi/MQTT
// RS-485 link to Engine-bay Node handled via UART2.

#include <SPI.h>
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();
float lastEgt = 0;

// -----------------------------------------------------------------------------
// Pin map for Qualia ESP32-S3 – change to suit your carrier wiring
// -----------------------------------------------------------------------------
#define PIN_SPI_MOSI  11   // Qualia breakout MOSI
#define PIN_SPI_MISO  13   // Qualia breakout MISO
#define PIN_SPI_CLK   12   // Qualia breakout SCLK
#define PIN_SPI_CS    10   // Chip-select from Intercept board
#define PIN_SPI_DRDY  9    // Data-ready interrupt from ADS131M08

#define PIN_RS485_RX  18   // UART2 RX
#define PIN_RS485_TX  17   // UART2 TX
#define PIN_RS485_EN  16   // Direction control

#define FRAME_MAX 32
uint8_t rxBuf[FRAME_MAX];
uint8_t rxPos = 0;
bool inFrame = false;

// CRC16 helper
uint16_t crc16_ccitt(const uint8_t *data, size_t len) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < len; i++) {
    crc ^= (uint16_t)data[i] << 8;
    for (int j = 0; j < 8; j++) {
      if (crc & 0x8000)
        crc = (crc << 1) ^ 0x1021;
      else
        crc <<= 1;
    }
  }
  return crc;
}

// -----------------------------------------------------------------------------
SPIClass spi(&SPI);
volatile bool adsDataReady = false;

void IRAM_ATTR onAdsDrdy() {
  adsDataReady = true;
}

void setup() {
  Serial.begin(115200);
  delay(500);

  // SPI setup
  spi.begin(PIN_SPI_CLK, PIN_SPI_MISO, PIN_SPI_MOSI, PIN_SPI_CS);
  pinMode(PIN_SPI_CS, OUTPUT);
  digitalWrite(PIN_SPI_CS, HIGH);

  // DRDY interrupt
  pinMode(PIN_SPI_DRDY, INPUT_PULLUP);
  attachInterrupt(PIN_SPI_DRDY, onAdsDrdy, FALLING);

  // RS-485 setup
  Serial2.begin(1000000, SERIAL_8N1, PIN_RS485_RX, PIN_RS485_TX);
  pinMode(PIN_RS485_EN, OUTPUT);
  digitalWrite(PIN_RS485_EN, LOW); // Receive mode

  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Dash Master v0.1", 10, 10, 2);

  Serial.println("Dash Master v0.1 – Starting up");
}

void loop() {
  // Read a frame when DRDY asserted (~2 kHz)
  if (adsDataReady) {
    adsDataReady = false;
    uint16_t raw[8];
    digitalWrite(PIN_SPI_CS, LOW);
    for (int i = 0; i < 8; i++) {
      raw[i] = spi.transfer16(0x0000);
    }
    digitalWrite(PIN_SPI_CS, HIGH);

    // Simple convert (placeholder) – each channel 16-bit signed
    float volts[8];
    for (int i = 0; i < 8; i++) {
      volts[i] = ((int16_t)raw[i]) * (5.0 / 32768.0);
    }

    // TODO: map volts[] to coolant, IAT, MAP, VF1, battery, etc.
    Serial.printf("THW %.2f °C, THA %.2f °C, PIM %.2f kPa\n", volts[0]*100, volts[1]*100, volts[2]*100); // placeholder scaling
  }

  // RS-485 byte stream
  while (Serial2.available()) {
    uint8_t b = Serial2.read();
    if (!inFrame) {
      if (b == 0x55) { inFrame = true; rxPos = 0; rxBuf[rxPos++] = b; }
    } else {
      rxBuf[rxPos++] = b;
      if (rxPos >= FRAME_MAX) inFrame = false; // overflow
      if (b == 0xAA && rxPos >= 7) {
        // frame complete
        uint8_t len = rxBuf[1];
        if (rxPos == len && len < FRAME_MAX) {
          uint16_t crc = rxBuf[len-3] | (rxBuf[len-2] << 8);
          uint16_t calc = crc16_ccitt(rxBuf, len-3);
          if (crc == calc) {
            uint8_t msg = rxBuf[2];
            if (msg == 0x01) {
              const uint8_t *p = &rxBuf[3];
              uint16_t seq = p[0] | (p[1] << 8);
              int16_t egt = p[2] | (p[3] << 8);
              uint16_t afr = p[4] | (p[5] << 8);
              uint16_t baro = p[6] | (p[7] << 8);
              int16_t pcb = p[8] | (p[9] << 8);
              Serial.printf("SEQ:%u EGT:%dC AFR:%.2f Baro:%uhPa PCB:%dC\n", seq, egt, afr/1000.0, baro, pcb);
              lastEgt = egt;
              tft.fillRect(0, 30, 240, 20, TFT_BLACK);
              tft.setCursor(10, 30); tft.printf("EGT: %d C", egt);
              tft.fillRect(0, 50, 240, 20, TFT_BLACK);
              tft.setCursor(10, 50); tft.printf("AFR: %.2f", afr/1000.0);
              tft.fillRect(0, 70, 240, 20, TFT_BLACK);
              tft.setCursor(10, 70); tft.printf("Baro: %u hPa", baro);
            }
          }
        }
        inFrame = false;
      }
    }
  }
}
