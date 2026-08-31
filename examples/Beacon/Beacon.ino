#include <BLE.h>

// Starts a BLE Beacon in the background, no further app work required after begin()

BLEBeacon beacon;
void setup() {
  BLE.begin();

  // Mash up Teensy's factory-programmed 64-bit unique ID and a common header
  // for our "UUID"
  uint8_t uuid[16];
  memcpy(uuid, "TEENSYBCON", 10);
  uint32_t id[2] = { HW_OCOTP_CFG0, HW_OCOTP_CFG1 };
  memcpy(uuid + 10, id, sizeof(id));
  memset(uuid + 14, 0, 2);

  // Define the beacon and start it
  beacon.setUUID(BLEUUID(uuid));
  beacon.setMajorMinor(6, 7);
  beacon.setTXPower(-44); // -44dbm @ 1M
  beacon.begin();
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // BLE.update() must run every loop() iteration to service the BTstack
  // HCI UART and run loop -- see README.md.
  BLE.update();

  // Otherwise, we can do whatever app we want, the beacon is handled automatically.
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  BLE.update();
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
}
