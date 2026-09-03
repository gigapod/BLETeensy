// This example shows writing your own class to run a BLE device.
// The class will get all the callbacks

// Send any byte over USB Serial to make it send "BONGO" to your attached BLE client

// For low-level BTstack/HCI traffic (packet-level detail, not needed for normal use),
// build with -DDEBUG_BLETEENSY -DDEBUG_BLETEENSY_PORT=Serial as extra build flags.

#include <BLE.h>

// Note that this class implements the BLEService and the Characteristic Callbacks so it's self-contained
// Also implements the server onConnect/disconnect callbacks
class BLECustomService : public BLEService, public BLECharacteristicCallbacks, public BLEServerCallbacks {
  public:
    // Generated via uuidgen or a web service
    static constexpr const char *SERVICE_UUID = "4ae60001-a1ad-46b0-8234-88a23ad055b9";
    static constexpr const char *CHARACTERISTIC_UUID_1 = "4ae60002-a1ad-46b0-8234-88a23ad055b9";
    static constexpr const char *CHARACTERISTIC_UUID_2 = "4ae60003-a1ad-46b0-8234-88a23ad055b9";

    // Be sure to call the BLEService constructor first
    BLECustomService() : BLEService(BLEUUID(SERVICE_UUID)) {
      // Create the characteristics
      c1 = new BLECharacteristic(BLEUUID(CHARACTERISTIC_UUID_1), BLEWrite, "Write to me and I'll send it back");
      c2 = new BLECharacteristic(BLEUUID(CHARACTERISTIC_UUID_2), BLERead | BLENotify, "Echoed back data");
      // Have the characteristic call this class when something happens
      c1->setCallbacks(this);
      c2->setCallbacks(this);
      // Give some default values
      c1->setValue(String("Write Me"));
      c2->setValue(String("Echo"));
      // Add them to the service
      addCharacteristic(c1);
      addCharacteristic(c2);
    }

    // Let the main app set the readable value
    void out(String str) {
      Serial.printf("[%lu] Notifying c2 <- \"%s\"\n", millis(), str.c_str());
      c2->setValue(str);
    }

    bool connected = false;

  private:
    // These implement the BLECharacteristicCallbacks
    void onWrite(BLECharacteristic *c) {
      // Do the right thing depending on what characteristic was just written
      if (c != c1) {
        return;  // Shouldn't ever happen
      }
      Serial.printf("[%lu] c1 written (%u bytes): \"%.*s\"\n", millis(), (unsigned)c->valueLen(), (int)c->valueLen(), (const char *)c->valueData());
      c2->setValue((const uint8_t *)c->valueData(), c->valueLen());
    }

    void onRead(BLECharacteristic *c) {
      Serial.printf("[%lu] %s read\n", millis(), (c == c1) ? "c1" : (c == c2 ? "c2" : "?"));
    }

    // BLEServerCallbacks
    void onConnect(BLEServer *s) {
      (void) s;
      connected = true;
      Serial.printf("[%lu] Client connected\n", millis());
    }

    void onDisconnect(BLEServer *s) {
      (void) s;
      connected = false;
      Serial.printf("[%lu] Client disconnected\n", millis());
    }

    BLECharacteristic *c1; // Writable
    BLECharacteristic *c2; // Readable
};

// Actual instance of the service
BLECustomService svc;

// Periodic "still alive" status line so it's obvious from the Serial Monitor
// whether the sketch is running/connected vs. hung or reset.
static uint32_t lastStatus = 0;
static const uint32_t STATUS_INTERVAL_MS = 5000;

void printStatus() {
  Serial.printf("[%lu] status: %s, addr %s\n",
                millis(),
                svc.connected ? "connected" : "advertising",
                BLE.address().toString().c_str());
}

void setup() {
  while (!Serial && millis() < 3000) {}  // give the USB serial monitor time to attach
  if (CrashReport) {
    // If the last run ended in a hard fault, print exactly where before continuing.
    Serial.print(CrashReport);
  }

  Serial.println("CustomService: starting BLE...");
  BLE.begin("TeensyBongo");
  Serial.printf("BLE address: %s\n", BLE.address().toString().c_str());

  BLE.server()->addService(&svc);
  BLE.server()->setCallbacks(&svc);  // so svc's onConnect()/onDisconnect() actually fire
  BLE.startAdvertising();
  Serial.println("Advertising as \"TeensyBongo\"");

  lastStatus = millis();
}

void loop() {
  // BLE.update() must run every loop() iteration to service the BTstack
  // HCI UART and run loop -- see README.md.
  BLE.update();

  if ((millis() - lastStatus) >= STATUS_INTERVAL_MS) {
    printStatus();
    lastStatus = millis();
  }

  if (Serial.available()) {
    while (Serial.available()) {
      Serial.read();
    }
    Serial.printf("[%lu] Serial input received, sending BONGO\n", millis());
    svc.out("BONGO!");
  }
}
