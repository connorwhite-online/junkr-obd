// Engine-bay Sensor Node v0.1 (ESP32-C3)
// Publishes dummy EGT / AFR / baro data over RS-485 using protocol in docs/RS485_PROTOCOL.md

#include <HardwareSerial.h>
#include <CRC16.h>

#define PIN_RS485_RX 3
#define PIN_RS485_TX 1
#define PIN_RS485_EN 2

HardwareSerial Modbus(1);

uint16_t seq = 0;

void rs485Write(uint8_t *buf, size_t len) {
  digitalWrite(PIN_RS485_EN, HIGH); // TX
  delayMicroseconds(2);
  Modbus.write(buf, len);
  Modbus.flush();
  delayMicroseconds(10);
  digitalWrite(PIN_RS485_EN, LOW);  // RX
}

void setup() {
  pinMode(PIN_RS485_EN, OUTPUT);
  digitalWrite(PIN_RS485_EN, LOW);
  Modbus.begin(1000000, SERIAL_8N1, PIN_RS485_RX, PIN_RS485_TX);
}

void loop() {
  // Dummy sensor values
  int16_t egt = 300 + (millis() / 100) % 200; // 300-500 °C swing
  uint16_t afr = 14700; // Lambda 1.0
  uint16_t baro = 1013; // hPa
  int16_t pcb = 45;     // °C

  uint8_t payload[10];
  payload[0] = seq & 0xFF;
  payload[1] = seq >> 8;
  payload[2] = egt & 0xFF;
  payload[3] = egt >> 8;
  payload[4] = afr & 0xFF;
  payload[5] = afr >> 8;
  payload[6] = baro & 0xFF;
  payload[7] = baro >> 8;
  payload[8] = pcb & 0xFF;
  payload[9] = pcb >> 8;

  uint8_t frame[17];
  frame[0] = 0x55;          // SOF
  frame[1] = 17;            // LEN
  frame[2] = 0x01;          // MSG id SensorFrame
  memcpy(&frame[3], payload, 10);

  uint16_t crc = CRC16.ccitt(&frame[0], 13); // up to last payload byte
  frame[13] = crc & 0xFF;
  frame[14] = crc >> 8;
  frame[15] = 0xAA;         // EOF

  rs485Write(frame, 16);

  seq++;
  delay(10); // 100 Hz
}
