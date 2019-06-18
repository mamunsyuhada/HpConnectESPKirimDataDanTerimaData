#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define LED_BUILTIN 2


//BLE Characterstuc
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
int txValue = 0;

//mendefinisikan service
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
//karakteristiknya, Receiver dan Transmitter
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E" //menerima data dari device-lain
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E" //mengirimkan data dari yang lain

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) { //Ketika dia sedang nyambung
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) { //Saat ngga nyambung
      deviceConnected = false;
    };
};


// Event ketika menerima data
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.print("------------------payload message : ");
        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }

        if (rxValue.find("off") != -1) {
          Serial.println("Lampu On !!");
          digitalWrite(LED_BUILTIN, HIGH);
        } else if (rxValue.find("on") != -1) {
          Serial.println("Lampu Off !!");
          digitalWrite(LED_BUILTIN, LOW);
        }

        Serial.println("\n End Received ");
      }
    }
};

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);

  //Buat ble device dengan nama "BLE-Mamun"
  BLEDevice::init("BLE-Mamun");
  //buat BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  // make ble service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  //bikin karacteristik BLE
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  //harus dipencet notify nya..
  pCharacteristic->addDescriptor(new BLE2902());

  //bikin rx characteristic-nya
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE // ngasih kolom inputdata di device ble yang terhubung biar bisa nulis
                                       );
  pCharacteristic->setCallbacks(new MyCallbacks());
  // run callback's service
  pService->start();
  pServer->getAdvertising()->start();
  Serial.println("Waiting for a client connection notify");
}

void loop() {
  //Kirim data ke device ble lain
  if (deviceConnected) {
    txValue = random(-20, 100);
    pCharacteristic->setValue(String(txValue).c_str());
    pCharacteristic->notify();
    Serial.println("kirim data : " + String(txValue));
    delay(500);
  }

}
