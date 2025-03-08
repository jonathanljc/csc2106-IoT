#include <M5StickCPlus.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEClient.h>
#include <BLEAdvertisedDevice.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

// Custom GATT Service and Characteristic UUIDs (must match the peripheral dongle)
#define CUSTOM_SERVICE_UUID "78563412-7856-3412-7856-341278563412"
#define CUSTOM_CHAR_UUID    "01efcdab-01ef-cdab-01ef-cdab01efcdab"

// Target device name advertised by the dongle
#define TARGET_DEVICE_NAME "Thread+BLE Device"

// IR configuration: using GPIO26 for the IR LED, NEC code example.
static const uint16_t kIrLedPin = 26;
static const uint32_t IR_CODE   = 0x20DF10EF;
static const uint16_t IR_BITS   = 32;

// Global IRsend object
IRsend irsend(kIrLedPin);

// BLE globals
static BLEAddress* pServerAddress      = nullptr;
static bool doConnect                  = false;
static bool connected                  = false;
static BLEClient* pClient             = nullptr;
static BLERemoteCharacteristic* pRemoteCharacteristic = nullptr;

// A flag indicating that we need to send IR (set in notify callback)
static bool needToSendIR = false;

// Callback for BLE scanning
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) override {
    Serial.print("Found device: ");
    Serial.println(advertisedDevice.toString().c_str());
    
    // Check if the device advertises the target name or our custom service UUID
    if ((advertisedDevice.haveName() && advertisedDevice.getName() == TARGET_DEVICE_NAME) ||
        (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(BLEUUID(CUSTOM_SERVICE_UUID)))) {
      Serial.println(">>> Target device found! Stopping scan.");
      pServerAddress = new BLEAddress(advertisedDevice.getAddress());
      doConnect = true;
      BLEDevice::getScan()->stop();
    }
  }
};

// Notification callback: called when a notification is received from the peripheral.
void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                    uint8_t* pData, size_t length, bool isNotify) {
  Serial.print("Notification received: ");
  String received;
  for (size_t i = 0; i < length; i++) {
    received += (char)pData[i];
  }
  Serial.println(received);

  // If the notification message is "Triggered", schedule IR transmission
  if (received == "Triggered") {
    Serial.println("Scheduling IR transmission from BLE notification.");
    needToSendIR = true;
  }
}

// Connect to the BLE peripheral and discover our custom service/characteristic.
bool connectToServer(BLEAddress pAddress) {
  Serial.print("Connecting to ");
  Serial.println(pAddress.toString().c_str());

  pClient = BLEDevice::createClient();
  if (!pClient->connect(pAddress)) {
    Serial.println("Failed to connect.");
    return false;
  }
  Serial.println(" - Connected to server");

  // Discover custom service.
  BLERemoteService* pRemoteService = pClient->getService(CUSTOM_SERVICE_UUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find service UUID: ");
    Serial.println(CUSTOM_SERVICE_UUID);
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found custom service");

  // Discover custom characteristic.
  pRemoteCharacteristic = pRemoteService->getCharacteristic(CUSTOM_CHAR_UUID);
  if (pRemoteCharacteristic == nullptr) {
    Serial.print("Failed to find characteristic UUID: ");
    Serial.println(CUSTOM_CHAR_UUID);
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found custom characteristic");

  // Register to receive notifications if supported.
  if (pRemoteCharacteristic->canNotify()) {
    pRemoteCharacteristic->registerForNotify(notifyCallback);
    Serial.println(" - Registered for notifications");
  }

  // Optionally, read the initial value.
  std::string value = pRemoteCharacteristic->readValue();
  Serial.print("Initial characteristic value: ");
  Serial.println(value.c_str());

  return true;
}

void setup() {
  // Increase CPU frequency to help with timing
  setCpuFrequencyMhz(240);

  M5.begin(); // Initialize M5StickC Plus hardware
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting BLE Client on M5StickC Plus with IR transmission...");

  // Initialize the IR transmitter.
  irsend.begin();
  Serial.println("IR transmitter initialized.");

  // Initialize BLE.
  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->start(30, false);  // Scan for 30 seconds
}

void loop() {
  // If a target device is found, attempt connection.
  if (doConnect) {
    if (connectToServer(*pServerAddress)) {
      Serial.println("Connected to BLE server.");
      connected = true;
    } else {
      Serial.println("Connection failed. Restarting scan...");
      BLEDevice::getScan()->start(30, false);
    }
    doConnect = false;
  }

  // If not connected, continue scanning.
  if (!connected) {
    BLEDevice::getScan()->start(30, false);
  }

  // Check if we need to send IR:
  if (needToSendIR) {
    Serial.println("Preparing to send IR code...");

    // Optionally pause or stop scanning to reduce CPU load interference
    BLEDevice::getScan()->stop();
    Serial.println("BLE scanning stopped briefly for IR transmission.");

    // Small delay to let BLE tasks settle
    delay(50);

    // Send IR code
    Serial.println("Sending IR signal now...");
    irsend.sendNEC(IR_CODE, IR_BITS);
    Serial.println("IR signal sent!");

    // Resume scanning if needed
    if (!connected) {
      Serial.println("Resuming BLE scanning...");
      BLEDevice::getScan()->start(30, false);
    }

    needToSendIR = false;
  }

  delay(1000);
}
