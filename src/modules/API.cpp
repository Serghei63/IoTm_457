#include "ESPConfiguration.h"

void* getAPI_Cron(String subtype, String params);
void* getAPI_Loging(String subtype, String params);
void* getAPI_LogingDaily(String subtype, String params);
void* getAPI_IoTMath(String subtype, String params);
void* getAPI_owmWeather(String subtype, String params);
void* getAPI_Timer(String subtype, String params);
void* getAPI_Variable(String subtype, String params);
void* getAPI_VButton(String subtype, String params);
void* getAPI_AnalogAdc(String subtype, String params);
void* getAPI_BL0937(String subtype, String params);
void* getAPI_UART(String subtype, String params);
void* getAPI_AnalogBtn(String subtype, String params);
void* getAPI_ButtonIn(String subtype, String params);
void* getAPI_ButtonOut(String subtype, String params);

void* getAPI(String subtype, String params) {
void* tmpAPI;
if ((tmpAPI = getAPI_Cron(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Loging(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_LogingDaily(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_IoTMath(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_owmWeather(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Timer(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Variable(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_VButton(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_AnalogAdc(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_BL0937(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_UART(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_AnalogBtn(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_ButtonIn(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_ButtonOut(subtype, params)) != nullptr) return tmpAPI;
return nullptr;
}