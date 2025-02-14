#pragma once

#include "Global.h"
#include "MqttClient.h"
boolean isNetworkActive();
uint8_t getNumAPClients();
#ifdef ESP8266
void routerConnect();
boolean RouterFind(std::vector<String> jArray);
#else
void handleScanResults();
void WiFiUtilsItit();
void connectToNextNetwork();
void checkConnection();
void ScanAsync();
bool startAPMode();
#endif
uint8_t RSSIquality();
//extern void wifiSignalInit();
#ifdef LIBRETINY
String httpGetString(HTTPClient &http);
#endif