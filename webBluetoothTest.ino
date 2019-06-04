#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/


#define SERVICE_UUID         "4fafc201-1fb5-459e-8fcc-c5c9c331914b" //サービス
#define CHARACTERISTIC1_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8" //文字列(R/W)
#define CHARACTERISTIC2_UUID "20cfa7a2-b074-45a0-8eae-57d1f3fb17fa" //カウンタ値(R/W)
#define CHARACTERISTIC3_UUID "685419f0-df21-427b-85a9-33253273dcde" //アナログ値(R)
//#define CHARACTERISTIC4_UUID "977e2ee4-8553-4a21-8d77-e67b04d0aee6" //タッチセンサ値(R)
#define CHARACTERISTIC5_UUID "7e8915d0-46fd-4de5-aa2b-5194c018c747" //全の値の通知(N)

// 圧力センサーを接続するピン
// bluetoothを使う場合、アナログ入力は３２～３９以外は使えないらしい。
// https://teratail.com/questions/158916
// https://trac.switch-science.com/wiki/esp32_tips
const int sensorPin = 32;
// タッチセンサーピン
const int touchPin = 4;
// LEDを接続するピン
const int ledPin = 2;

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic1 = NULL;
BLECharacteristic* pCharacteristic2 = NULL;
BLECharacteristic* pCharacteristic3 = NULL;
//BLECharacteristic* pCharacteristic4 = NULL;
BLECharacteristic* pCharacteristic5 = NULL;
bool deviceConnected = false;
uint32_t count = 0; //カウンタ初期値
uint32_t analogVal = 0;
uint32_t touchVal = 0;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("connect");
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("disconnect");
      deviceConnected = false;
    }
};

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);

  BLEDevice::init("testESP32");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  pCharacteristic1 = pService->createCharacteristic(
                                         CHARACTERISTIC1_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE |
                                         BLECharacteristic::PROPERTY_NOTIFY |
                                         BLECharacteristic::PROPERTY_INDICATE
                                       );
  pCharacteristic1->addDescriptor(new BLE2902());
  
  pCharacteristic2 = pService->createCharacteristic(
                                         CHARACTERISTIC2_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE |
                                         BLECharacteristic::PROPERTY_NOTIFY |
                                         BLECharacteristic::PROPERTY_INDICATE
                                       );
  pCharacteristic2->addDescriptor(new BLE2902());
  
  pCharacteristic3 = pService->createCharacteristic(
                                         CHARACTERISTIC3_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE |
                                         BLECharacteristic::PROPERTY_NOTIFY |
                                         BLECharacteristic::PROPERTY_INDICATE
                                       );
  pCharacteristic3->addDescriptor(new BLE2902());
  
//  pCharacteristic4 = pService->createCharacteristic(
//                                         CHARACTERISTIC4_UUID,
//                                         BLECharacteristic::PROPERTY_READ |
//                                         BLECharacteristic::PROPERTY_WRITE |
//                                         BLECharacteristic::PROPERTY_NOTIFY |
//                                         BLECharacteristic::PROPERTY_INDICATE
//                                       );
//  pCharacteristic4->addDescriptor(new BLE2902());

  pCharacteristic5 = pService->createCharacteristic(
                                         CHARACTERISTIC5_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE |
                                         BLECharacteristic::PROPERTY_NOTIFY |
                                         BLECharacteristic::PROPERTY_INDICATE
                                       );
  pCharacteristic5->addDescriptor(new BLE2902());

  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();

  pCharacteristic1->setValue("Hello World!");
  pCharacteristic2->setValue((uint8_t*)&count, 4);
  pCharacteristic3->setValue((uint8_t*)&analogVal, 4);
//  pCharacteristic4->setValue((uint8_t*)&touchVal, 4);
  pCharacteristic5->setValue("xxxx");

  for(int i=1;i<=3;i++) {
    digitalWrite(ledPin, HIGH);
    delay(200);
    digitalWrite(ledPin, LOW);
    delay(200);
  }
}

void loop() {
  // カウンタ
  count++;
  // アナログ入力(0-4095)
  int analogValue = analogRead(sensorPin);
  Serial.println("analog = " + String(analogValue));
  // タッチセンサー入力
  int touchValue = touchRead(touchPin);
  Serial.println("touch = " + String(touchValue));

  if (deviceConnected) {
    // chara3にアナログ値を書き込み
    analogVal=analogValue;
    pCharacteristic3->setValue((uint8_t*)&analogVal, 4);

//    // chara4にタッチセンサ値を書き込み
//    touchVal=touchValue;
//    pCharacteristic4->setValue((uint8_t*)&touchVal, 4);

    // chara1取得
    std::string value1 = pCharacteristic1->getValue();
    Serial.println(value1.c_str());

    // chara2の値を読みだしてuint32_t型に変換後、数字に変換
    std::string v = pCharacteristic2->getValue();
    count=(uint8_t)v[0]+(uint8_t)v[1]*256+(uint8_t)v[2]*65536+(uint8_t)v[3]*16777216;
    String countStr = String(count);
    std::string value2 = countStr.c_str();
    Serial.println(value2.c_str());
    // カウンタをインクリメントしてchara2に書き込み
    count++;
    pCharacteristic2->setValue((uint8_t*)&count, 4);

    // アナログ値を数字に変換
    analogVal=analogValue;
    String analogStr = String(analogVal);
    std::string value3 = analogStr.c_str();
    Serial.println(value3.c_str());

//    // タッチセンサー値を数字に変換
//    touchVal=touchValue;
//    String touchStr = String(touchVal);
//    std::string value4 = touchStr.c_str();
//    Serial.println(value4.c_str());

    // chara5に「chara1+chara2+chara3+char4」をセット
//    std::string value5 = value1 + value2 + value3 + value4;
    std::string value5 = value1 + " " + value2 + " " + value3;
    pCharacteristic5->setValue(value5);
    Serial.println(value5.c_str());
    value5 = pCharacteristic5->getValue();
    Serial.println(value5.c_str());

    // chara5をnotify
    pCharacteristic5->notify();
    //pCharacteristic5->indicate();

    digitalWrite(ledPin, HIGH);
    delay(200);
    digitalWrite(ledPin, LOW);
    delay(200);
  } else {
    digitalWrite(ledPin, HIGH);
    delay(10);
    digitalWrite(ledPin, LOW);
    delay(500);
  }
}
