#include "ESPConfiguration.h"

void* getAPI_Benchmark(String subtype, String params);
void* getAPI_Cron(String subtype, String params);
void* getAPI_Timer(String subtype, String params);
void* getAPI_Variable(String subtype, String params);
void* getAPI_VButton(String subtype, String params);
void* getAPI_UpdateServer(String subtype, String params);
void* getAPI_ModbusRTUasync(String subtype, String params);
void* getAPI_RTC(String subtype, String params);
void* getAPI_Telegram_v2(String subtype, String params);

void* getAPI(String subtype, String params) {
void* tmpAPI;
if ((tmpAPI = getAPI_Benchmark(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Cron(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Timer(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Variable(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_VButton(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_UpdateServer(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_ModbusRTUasync(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_RTC(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Telegram_v2(subtype, params)) != nullptr) return tmpAPI;
return nullptr;
}