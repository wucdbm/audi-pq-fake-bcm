#include <CAN.h>

uint8_t payload397[8] = {0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t counter397 = 0; // Increments 0 to 15 (lower 4 bits of Byte 1) 0x00 to 0x0F
unsigned long last_0x397_tx = 0;
const unsigned long tx_interval_0x397 = 100; // BCM sends every 100 milliseconds

uint8_t payload531[4] = {0xC3, 0x00, 0x00, 0x00};
uint8_t counter531 = 0x00; // Increments by 0x10 from 0x00 to 0xF0
unsigned long last_0x531_tx = 0;
const unsigned long tx_interval_0x531 = 50; // BCM sends every 50 milliseconds

void setup() {
  Serial.begin(9600);

  while (!Serial);

  // Convenience CAN Bus runs at 100 kbps
  if (!CAN.begin(100E3)) {
    Serial.println("Starting CAN failed!");
    while (1);
  }

  Serial.println("Listening");
}

void loop() {
  send_0x397();
  send_0x531();
}

void send_0x397() {
  unsigned long currentMillis = millis();

  if (currentMillis - last_0x397_tx < tx_interval_0x397) {
    return;
  }

  last_0x397_tx = currentMillis;

  // 1. Keep upper nibble of Byte 1, inject the 4-bit rolling counter
  uint8_t statusBits = payload397[1] & 0xF0;
  payload397[1] = statusBits | (counter397 & 0x0F);

  // 2. Calculate Checksum (Byte 0 = Byte 1 ^ 0x01)
  payload397[0] = payload397[1] ^ 0x01;

  // 3. Send
  CAN.beginPacket(0x397);
  Serial.print("Send 0x397 # ");
  for (int i = 0; i < sizeof(payload397); i++) {
    CAN.write(payload397[i]);
      Serial.print(payload397[i] < 16 ? "0" : "");
      Serial.print(payload397[i], HEX);
      Serial.print(" ");
  }
  Serial.println("");
  CAN.endPacket();

  // 4. Advance and wrap the counter (0-15 loop)
  counter397 = (counter397 + 1) % 16;
}

void send_0x531() {
  unsigned long currentMillis = millis();

  if (currentMillis - last_0x531_tx < tx_interval_0x531) {
    return;
  }

  last_0x531_tx = currentMillis;

  // 1. Load the current 0x10 step counter into Byte 2
  payload531[2] = counter531;

  // 2. Calculate Checksum (Byte 3 = Byte 2 ^ 0xC3)
  payload531[3] = payload531[2] ^ 0xC3;

  // 3. Send
  CAN.beginPacket(0x531);
  Serial.print("Send 0x531 # ");
  for (int i = 0; i < sizeof(payload531); i++) {
    CAN.write(payload531[i]);
      Serial.print(payload531[i] < 16 ? "0" : "");
      Serial.print(payload531[i], HEX);
      Serial.print(" ");
  }
  Serial.println("");
  CAN.endPacket();

  // 4. Advance counter by 0x10 (auto-wraps to 0x00 when it exceeds 0xF0)
  counter531 += 0x10;
}