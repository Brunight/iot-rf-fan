#ifndef RFMANAGER_H
#define RFMANAGER_H

#define RF_TRANSMITTER_PIN 14

#include "WebServer/WebServer.h"
#include <ArduinoHA.h>
#include <ArduinoJson.h>
#include <RCSwitch.h>

class RfManager {
public:
  RfManager(int pin, int protocol);
  RfManager(int pin, int protocol, int defaultLength);
  void begin();
  void loop();
  void sendRfCode(unsigned long code);
  void sendRfCode(unsigned long code, int length);

private:
  int _protocol;
  int _pin;
  int _defaultLength = 32;
  unsigned long _lastRfSent = 0;
  RCSwitch* _mySwitch;
  JsonDocument rfQueueDocument;
  JsonArray rfQueue = rfQueueDocument.to<JsonArray>();
};

#endif