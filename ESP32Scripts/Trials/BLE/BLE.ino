#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

BLEServer* server = nullptr;
BLECharacteristic* characteristic = nullptr;

class RayMinderCharacteristicCallbacks: public BLECharacteristicCallbacks {
  // Receive message from phone
  void onWrite(BLECharacteristic* characteristic) override {
    String message = (String)(characteristic->getValue().c_str());
    if (message.length() > 0) {
      Serial.print("Received message: ");
      Serial.println(message.c_str());
    }
  }
};

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("Starting...");

  // Initialaze BLE
  BLEDevice::init("ESP32-C3");

  // Create BLE server
  server = BLEDevice::createServer();

  BLESecurity *pSecurity = new BLESecurity();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_NO_BOND);

  // Creaet service
  BLEService *service = server->createService("12345678-1234-1234-1234-123456789012");

  // Create characteristic
  characteristic = service->createCharacteristic(
    "12341234-1234-1234-1234-123412341234",
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY
  );

  characteristic->setCallbacks(new RayMinderCharacteristicCallbacks());
  characteristic->setValue("");

  // Start service
  service->start();
  server->getAdvertising()->start();

  Serial.println("BLE started, now connecting to the phone.");
}

void loop() {
  if (Serial.available()) readMessages();
  delay(200);
}

String message;
char c;

// Read & decode messages send by phone
void readMessages() {
  message = "";

  while (Serial.available()) {
    c = Serial.read();

    if (c == '\n' || c == '\0') {
      break;
    }

    message += c;
  }

  Serial.print("Sending message: ");
  Serial.println(message);

  characteristic->setValue(message.c_str());
  characteristic->notify();
}