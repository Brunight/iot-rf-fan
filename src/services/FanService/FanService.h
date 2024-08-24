#ifndef FANSERVICE_H
#define FANSERVICE_H

#define RF_TRANSMITTER_PIN 14

#include "WebServer/WebServer.h"
#include <ArduinoHA.h>

class FanService {
public:
  FanService(WebServer* webserver);
  void begin();
  void loop();
  
private:
  WebServer* _webserver;
  static void onLightStateCommand(bool state, HALight *sender);
  static void onFanStateCommand(bool state, HAFan *sender);
  static void onFanSpeedCommand(uint16_t speed, HAFan *sender);
  static void onResetCommand(HAButton *sender);
  static bool lightState;
  static bool fanState;
  static int fanSpeed;
};

#endif