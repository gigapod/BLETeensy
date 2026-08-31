#include <BLE.h>

BLEServiceUART uart;
BLEServiceBattery batt;

void setup() {
  delay(5000);
  BLE.begin("TeensyUART");
  BLE.server()->addService(&batt);
  BLE.server()->addService(&uart);
  BLE.startAdvertising();
  uart.setAutoflush(50);
}

int cnt = 0;
void loop() {
  // BLE.update() must run every loop() iteration to service the BTstack
  // HCI UART and run loop -- see README.md.
  BLE.update();

  while (uart.available()) {
    Serial.println(uart.read());
  }
  uart.println(cnt++);
  uart.println("This text is very long and should require multiple writes to happen");
  delay(1000);
}
