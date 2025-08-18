#include "ESPConfiguration.h"

void* getAPI_Cron(String subtype, String params);
void* getAPI_Ping(String subtype, String params);
void* getAPI_Timer(String subtype, String params);
void* getAPI_UpdateServer(String subtype, String params);
void* getAPI_Variable(String subtype, String params);
void* getAPI_VButton(String subtype, String params);
void* getAPI_Ds18b20(String subtype, String params);
void* getAPI_ModbusRTUasync(String subtype, String params);
void* getAPI_RTC(String subtype, String params);
void* getAPI_UART(String subtype, String params);
void* getAPI_ButtonIn(String subtype, String params);
void* getAPI_ButtonOut(String subtype, String params);
void* getAPI_TelegramLT(String subtype, String params);

void* getAPI(String subtype, String params) {
void* tmpAPI; void* foundAPI = nullptr;
if ((tmpAPI = getAPI_Cron(subtype, params)) != nullptr) foundAPI = tmpAPI;
if ((tmpAPI = getAPI_Ping(subtype, params)) != nullptr) foundAPI = tmpAPI;
if ((tmpAPI = getAPI_Timer(subtype, params)) != nullptr) foundAPI = tmpAPI;
if ((tmpAPI = getAPI_UpdateServer(subtype, params)) != nullptr) foundAPI = tmpAPI;
if ((tmpAPI = getAPI_Variable(subtype, params)) != nullptr) foundAPI = tmpAPI;
if ((tmpAPI = getAPI_VButton(subtype, params)) != nullptr) foundAPI = tmpAPI;
if ((tmpAPI = getAPI_Ds18b20(subtype, params)) != nullptr) foundAPI = tmpAPI;
if ((tmpAPI = getAPI_ModbusRTUasync(subtype, params)) != nullptr) foundAPI = tmpAPI;
if ((tmpAPI = getAPI_RTC(subtype, params)) != nullptr) foundAPI = tmpAPI;
if ((tmpAPI = getAPI_UART(subtype, params)) != nullptr) foundAPI = tmpAPI;
if ((tmpAPI = getAPI_ButtonIn(subtype, params)) != nullptr) foundAPI = tmpAPI;
if ((tmpAPI = getAPI_ButtonOut(subtype, params)) != nullptr) foundAPI = tmpAPI;
if ((tmpAPI = getAPI_TelegramLT(subtype, params)) != nullptr) foundAPI = tmpAPI;
return foundAPI;
}