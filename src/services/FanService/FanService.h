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
  // All of those need to be static because the HASS lib doesn't allow to pass a pointer to a member function (lambda with capture)
  static WebServer* _webserver;
  static void onLightStateCommand(bool state, HALight *sender);
  static void onFanStateCommand(bool state, HAFan *sender);
  static void onFanSpeedCommand(uint16_t speed, HAFan *sender);
  static void onResetCommand(HAButton *sender);
  static bool lightState;
  static bool fanState;
  static int fanSpeed;
  static String getStatus();
  static void broadcastStatus();
};

#endif