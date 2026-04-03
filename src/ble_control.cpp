#include "ble_control.h"
#include "emotion_registry.h"
#include <NimBLEDevice.h>

BleControl bleControl;

static BleEmotionFn s_callback = nullptr;
static uint8_t s_currentEmotion = 0;

#define SERVICE_UUID "face0001-0000-1000-8000-00805f9b34fb"
#define CHAR_UUID    "face0002-0000-1000-8000-00805f9b34fb"

class EmotionCallbacks : public NimBLECharacteristicCallbacks {
  // Validates the written emotion ID and fires the registered emotion callback if valid.
  void onWrite(NimBLECharacteristic* pChar) override {
    std::string val = pChar->getValue();
    if (val.length() >= 1) {
      uint8_t id = (uint8_t)val[0];
      if (emotionRegistry.get((EmotionState)id) != nullptr && s_callback) {
        s_callback((EmotionState)id);
      }
    }
  }
  // Responds to a BLE read request with the current emotion ID byte.
  void onRead(NimBLECharacteristic* pChar) override {
    pChar->setValue(&s_currentEmotion, 1);
  }
};

static EmotionCallbacks s_charCallbacks;

// Initializes NimBLE, creates the emotion service and characteristic, and starts advertising as "SANGI".
void BleControl::init(BleEmotionFn onEmotion) {
  onEmotion_ = onEmotion;
  s_callback = onEmotion;

  NimBLEDevice::init("SANGI");
  NimBLEDevice::setPower(ESP_PWR_LVL_P3);

  NimBLEServer* pServer = NimBLEDevice::createServer();
  NimBLEService* pService = pServer->createService(SERVICE_UUID);
  NimBLECharacteristic* pChar = pService->createCharacteristic(
      CHAR_UUID,
      NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  pChar->setCallbacks(&s_charCallbacks);
  pService->start();

  NimBLEAdvertising* pAdv = NimBLEDevice::getAdvertising();
  pAdv->addServiceUUID(SERVICE_UUID);
  pAdv->setScanResponse(true);
  pAdv->start();

  Serial.println("BLE: advertising as 'SANGI'");
}

// Updates the cached current emotion ID so subsequent BLE reads reflect the latest state.
void BleControl::updateCurrentEmotion(uint8_t emotionId) {
  s_currentEmotion = emotionId;
}
