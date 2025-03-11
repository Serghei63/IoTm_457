#include "Global.h"
#include "classes/IoTItem.h"
#include <ArduinoJson.h>

long prevWeatherMillis = millis() - 60001;
StaticJsonDocument<JSON_BUFFER_SIZE * 2> Weatherdoc1;

extern IoTGpio IoTgpio;
class Wea : public IoTItem
{
private:
//описание параметров передаваемых из настроек датчика из веба

    String _location;
    String _param;
    //long interval;
    String _API_key;
    String _ID_sity;

public:
    Wea(String parameters) : IoTItem(parameters)
    {
        _API_key = jsonReadStr(parameters, "API_key");
        _ID_sity = jsonReadStr(parameters, "ID_sity");
        _location = jsonReadStr(parameters, "location");
        _param = jsonReadStr(parameters, "param");
       // jsonRead(parameters, F("int"), interval);
       // interval = interval * 1000 * 60 * 60; // интервал проверки погоды в часах

       long interval;
       jsonRead(parameters, F("int"), interval);
       interval = interval * 1000 * 60 * 60;  // интервал проверки погоды в часах
    }

    void getWea()
    {
        String ret;

        if (WiFi.status() == WL_CONNECTED)
        {
            // char c;
            String payload;
            WiFiClient client;
            HTTPClient http;
            //http.begin(client, "http://api.openweathermap.org/data/2.5/weather?id=524901&appid=6a4ba421859c9f4166697758b68d889b&units=metric");
            http.begin(client, "http://api.openweathermap.org/data/2.5/weather?id=" + _ID_sity + "&appid=" + _API_key + "&units=metric");
            http.addHeader("Content-Type", "application/x-www-form-urlencoded");
            String httpRequestData = "loc=" + _location;
            int httpResponseCode = http.POST(httpRequestData);
           // int httpResponseCode = http.GET(httpRequestData);
            if (httpResponseCode > 0)
            {
                ret = httpResponseCode;

                if (httpResponseCode == HTTP_CODE_OK)
                {
                    payload = http.getString();

                    deserializeJson(Weatherdoc1, payload);
                     ret += payload;
                }
            }
            else
            {
                ret = http.errorToString(httpResponseCode).c_str();
            }
            SerialPrint("<-", F("getWea"), httpRequestData);
            SerialPrint("->", F("getWea"), "server: " + ret);

            http.end();
        }
    }

    void doByInterval()
    {

        if (prevWeatherMillis + 60000 < millis())
        {
            getWea();
            prevWeatherMillis = millis();
        }
        if (jsonReadStr(Weatherdoc1["main"], "temp", true) != "null")
        {
            if (_param == "temp")
            {
                value.valS = jsonReadStr(Weatherdoc1["main"], "temp", true);
            }
            if (_param == "pressure")
            {
                value.valS = jsonReadStr(Weatherdoc1["main"], "pressure", true);
            }
            if (_param == "humidity")
            {
                value.valS = jsonReadStr(Weatherdoc1["main"], "humidity", true);
            }
            if (_param == "speed")
            {
                value.valS = jsonReadStr(Weatherdoc1["wind"], "speed", true);
            }
            if (_param == "deg")
            {
                value.valS = jsonReadStr(Weatherdoc1["wind"], "deg", true);
            }
            if (_param == "all")
            {
                value.valS = jsonReadStr(Weatherdoc1["clouds"], "all", true);
            }
            if (_param == "main")
            {
                value.valS = jsonReadStr(Weatherdoc1["weather"][0], "main", true);
            }
            if (_param == "description")
            {
                value.valS = jsonReadStr(Weatherdoc1["weather"][0], "description", true);
            }
            if (_param == "icon")
            {
                value.valS = jsonReadStr(Weatherdoc1["weather"][0], "icon", true);
            }           
            if (_param == "sunrise")
            {
                value.valS = jsonReadStr(Weatherdoc1["sys"], "sunrise", true);
            }
            if (_param == "sunset")
            {
                value.valS = jsonReadStr(Weatherdoc1["sys"], "sunset", true);
            }

             // value.isDecimal = false; 

            regEvent(value.valS, "Wea");
        }
    }

    IoTValue execute(String command, std::vector<IoTValue> &param)
    {
        if (command == "get")
        {
            getWea();
            doByInterval();
        }

        return {};
    }

    ~Wea(){};
};

void *getAPI_Wea(String subtype, String param)
{
    if (subtype == F("Wea"))
    {
        return new Wea(param);
    }
    else
    {
        return nullptr;
    }
}
