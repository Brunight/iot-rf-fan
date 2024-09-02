#include "WebServer/WebServer.h"
#include "services/FanService/FanService.h"
#include "RfManager/RfManager.h"
#include "utils/boolToString.cpp"
#include <ArduinoHA.h>
#include <ArduinoJson.h>
#include "secrets.h"

WiFiClient haClient;
HADevice device;
#ifdef SECRET_MQTT_BROKER_IP
  HAMqtt mqtt(haClient, device);
#endif

String haName = "esp8266";
HAFan fan("ventilador", HAFan::SpeedsFeature);
HALight light("luz_ventilador");
HAButton button("reset_ventilador");

RfManager rfManager(RF_TRANSMITTER_PIN, 6);

WebServer* FanService::_webserver;

FanService::FanService(WebServer* webserver) {
  this->_webserver = webserver;
}

void FanService::begin() {
  rfManager.begin();

  byte mac[WL_MAC_ADDR_LENGTH];
  WiFi.macAddress(mac);
  device.setUniqueId(mac, sizeof(mac));
  device.setName("NodeMCU");
  device.setSoftwareVersion("1.0.0");
  device.enableSharedAvailability();

  // Fan
  fan.setName("Ventilador");
  fan.setSpeedRangeMin(1);
  fan.setSpeedRangeMax(2);
  fan.onStateCommand(onFanStateCommand);
  fan.onSpeedCommand(onFanSpeedCommand);
  fan.setCurrentState(false);
  fan.setCurrentSpeed((unsigned int)2);

  // Fan light bulb
  light.onStateCommand(onLightStateCommand);
  light.setName("Luz Ventilador");
  light.setCurrentState(true);

  // RESET Fan button
  button.onCommand(onResetCommand);
  button.setName("RESET Ventilador");
  button.setDeviceClass("restart");
  
  #ifdef SECRET_MQTT_BROKER_IP
    mqtt.begin(SECRET_MQTT_BROKER_IP);
  #endif
  
  _webserver->on("/api/status", HTTP_GET, REQUIRE_AUTH, [this](AsyncWebServerRequest* request) {
    request->send(200, "application/json", getStatus());
  });

  _webserver->on("/api/light/toggle", HTTP_GET, REQUIRE_AUTH, [this](AsyncWebServerRequest* request) {
    onLightStateCommand(!lightState, &light);
    request->send(200, "application/json", "{\"value\": " + boolToString(lightState) + "}");
  });
  _webserver->on("/api/light/toggle", HTTP_POST, REQUIRE_AUTH, [this](AsyncWebServerRequest* request) {
    onLightStateCommand(!lightState, &light);
    request->send(200, "application/json", "{\"value\": " + boolToString(lightState) + "}");
  });
  _webserver->on("/api/fan/toggle", HTTP_GET, REQUIRE_AUTH, [this](AsyncWebServerRequest* request) {
    onFanStateCommand(!fanState, &fan);
    request->send(200, "application/json", "{\"value\": " + boolToString(fanState) + "}");
  });
  _webserver->on("/api/fan/toggle", HTTP_POST, REQUIRE_AUTH, [this](AsyncWebServerRequest* request) {
    onFanStateCommand(!fanState, &fan);
    request->send(200, "application/json", "{\"value\": " + boolToString(fanState) + "}");
  });
  _webserver->on("/api/fan/speed/toggle", HTTP_POST, REQUIRE_AUTH, [this](AsyncWebServerRequest* request) {
    unsigned int newSpeed = fanSpeed == 0 ? 2 : fanSpeed - 1;
    onFanSpeedCommand(newSpeed, &fan);
    request->send(200, "application/json", "{\"value\": " + boolToString(newSpeed) + "}");
  });
  _webserver->on("/api/reset", HTTP_POST, REQUIRE_AUTH, [this](AsyncWebServerRequest* request) {
    onResetCommand(&button);
    request->send(201);
  });
}

bool FanService::lightState = true;
bool FanService::fanState = false;
int FanService::fanSpeed = 2;

void FanService::onLightStateCommand(bool state, HALight *sender) {
  if (lightState == state) {
    return;
  }

  lightState = state;
  sender->setState(state);
  broadcastStatus();
  rfManager.sendRfCode(3603628053); // FAN LIGHT ON/OFF
};

void FanService::onFanStateCommand(bool state, HAFan *sender) {
  if (fanState == state) {
    return;
  }

  fanState = state;
  sender->setState(state);
  broadcastStatus();
  rfManager.sendRfCode(3603628069); // FAN ON/OFF
};

void FanService::onFanSpeedCommand(uint16_t speed, HAFan *sender) {
  // Fan speed can only be set if the fan is on
  if (!fanState) {
    return;
  }

  if (fanSpeed == speed) {
    return;
  }


  sender->setSpeed(speed);
  broadcastStatus();
  while (speed != fanSpeed) {
    rfManager.sendRfCode(3603628101); // FAN SPEED TOGGLE
    unsigned int newSpeed = fanSpeed == 0 ? 2 : fanSpeed - 1;
    fanSpeed = newSpeed;
  }
}

void FanService::onResetCommand(HAButton *sender) {
  fanState = false;
  lightState = true;
  fanSpeed = 2;
  fan.setState(false);
  fan.setSpeed((unsigned int)2);
  light.setState(true);
  broadcastStatus();
}

void FanService::loop() {
  rfManager.loop();
  #ifdef SECRET_MQTT_BROKER_IP
    mqtt.loop();
  #endif
}

String FanService::getStatus() {
  return "{\"light\":" + boolToString(lightState) + ",\"fan\":" + boolToString(fanState) + ",\"speed\":" + boolToString(fanSpeed) + "}";
}

void FanService::broadcastStatus() {
  _webserver->ws.textAll(getStatus());
}