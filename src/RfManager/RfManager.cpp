#include "RfManager/RfManager.h"

RfManager::RfManager(int pin, int protocol) {
  _pin = pin;
  _protocol = protocol;
}
RfManager::RfManager(int pin, int protocol, int defaultLength) {
  _pin = pin;
  _protocol = protocol;
  _defaultLength = defaultLength;
}

void RfManager::begin() {
  _mySwitch = new RCSwitch();
  _mySwitch->enableTransmit(_pin);
  _mySwitch->setProtocol(_protocol);
}

void RfManager::loop() {
  if (rfQueue.size() > 0 && millis() - _lastRfSent > 500) {
    unsigned long code = rfQueue[0]["code"];
    int length = rfQueue[0]["length"];
    _mySwitch->send(code, length);
    rfQueue.remove(0);
    _lastRfSent = millis();
  }
}

void RfManager::sendRfCode(unsigned long code) {
  sendRfCode(code, _defaultLength);
}

void RfManager::sendRfCode(unsigned long code, int length) {
  JsonDocument jsonDocument;
  JsonObject data = jsonDocument.to<JsonObject>();
  data["code"] = code;
  data["length"] = length;
  rfQueue.add(data);
}
