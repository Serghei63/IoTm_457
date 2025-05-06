#pragma once

#include "Global.h"
#include "MqttClient.h"
boolean isNetworkActive();
uint8_t getNumAPClients();
bool startAPMode();
#ifndef WIFI_ASYNC
void routerConnect();
boolean RouterFind(std::vector<String> jArray);
#else
void handleScanResults();
void WiFiUtilsItit();
void connectToNextNetwork();
void checkConnection();
void ScanAsync();

#endif
uint8_t RSSIquality();
//extern void wifiSignalInit();
#ifdef LIBRETINY
String httpGetString(HTTPClient &http);
#endif