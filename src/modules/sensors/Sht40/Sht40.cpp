#include "Global.h"
#include "classes/IoTItem.h"


#include "Wire.h"
#include <SHT40AD1BSensor.h>

SHT40AD1BSensor sensor(&Wire);
float tmp,hum;

extern IoTGpio IoTgpio;

class Sht40t : public IoTItem {

    private:
    uint8_t _addr = 0;

   public:
    Sht40t(String parameters): IoTItem(parameters) {
    {
        String sAddr;
        jsonRead(parameters, "addr", sAddr);
        if (sAddr == "")
            scanI2C();
        else
            _addr = hexStringToUint8(sAddr);
    }

     }
    
    void doByInterval() {

        value.valD = sensor.GetTemperature(&tmp);

        SerialPrint("i", "Sensor Sht40t", "OK");

        if (value.valD > -46.85F) regEvent(value.valD, "Sht40t");     // TODO: найти способ понимания ошибки получения данных
            else SerialPrint("E", "Sensor Sht40t", "Error", _id);  
       
    }
    ~Sht40t() {};
};

class Sht40h : public IoTItem {

    private:
    uint8_t _addr = 0;

   public:
    Sht40h(String parameters): IoTItem(parameters) {
            {
        String sAddr;
        jsonRead(parameters, "addr", sAddr);
        if (sAddr == "")
            scanI2C();
        else
            _addr = hexStringToUint8(sAddr);
    }
     }
    
    void doByInterval() {
   
        value.valD = sensor.GetHumidity(&hum); 
        SerialPrint("i", "Sensor Sht40h", "OK");
        if (value.valD != -6) regEvent(value.valD, "Sht40h");    // TODO: найти способ понимания ошибки получения данных
            else SerialPrint("E", "Sensor Sht40h", "Error", _id);
         
    }
    ~Sht40h() {};
};


void* getAPI_Sht40(String subtype, String param) {
    if (subtype == F("Sht40t")){
        return new Sht40t(param);
        }
        if (subtype == F("Sht40h")) {
            return new Sht40h(param);
    } else {
        return nullptr;
    }
}
