/******************************************************************
  Used INA Current Sensor
  Support for INA3221
  https://github.com/Tinyu-Zhao/INA3221

  adapted for version 4dev @Mit4bmw
 ******************************************************************/

#include "Global.h"
#include "classes/IoTItem.h"

#include <Wire.h>
#include <INA3221.h>
#include <map>



// Структура для хранения настроек датчика
struct Ina3221Value
{
    float shunt = 0;
    float maxV = 0;
};

// глобальные списки необходимы для хранения Модуля Настроек. Ключ - адрес
std::map<uint8_t, Ina3221Value *> ina3221SettingArray;

// глобальные списки необходимы для хранения объектов  используемых разными датчиками из модуля. Ключ - адрес
std::map<uint8_t, INA3221 *> ina3221Array;

// Функция инициализации библиотечного класса, возвращает Единстрвенный указать на библиотеку
INA3221 *instanceIna3221(uint8_t ADDR)
{
    /** default I2C address **/
    if (ADDR == 0)
        ADDR = 0x40; // 1000000 (A0+A1=GND)

    // учитываем, что библиотека может работать с несколькими линиями на разных пинах, поэтому инициируем библиотеку, если линия ранее не использовалась
    if (ina3221Array.find(ADDR) == ina3221Array.end())
    {
        if (ina3221SettingArray.find(ADDR) != ina3221SettingArray.end())
           // ina3221Array[ADDR] = new INA3221(ina3221SettingArray[ADDR]->shunt, ina3221SettingArray[ADDR]->maxV, (uint8_t)ADDR);
       // else
          //  ina3221Array[ADDR] = new INA3221(0.1f, 0.8f, (uint8_t)ADDR); // Стандартные значения для модуля INA226 (0.1 Ом, 0.8А, адрес 0x40)
        ina3221Array[ADDR]->begin();
        ina3221Array[ADDR]->reset();
            // ina3221ValueArray[ADDR] = new Ina3221Value;
    }
    return ina3221Array[ADDR];
}

class Ina3221voltage1 : public IoTItem
{
private:
    uint8_t _addr = 0;
    uint8_t _canel = 0;
public:
    Ina3221voltage1(String parameters) : IoTItem(parameters)
    {
        String sAddr;
        int _canel;
        jsonRead(parameters, "addr", sAddr);
        if (sAddr == "")
            scanI2C();
        else
            _addr = hexStringToUint8(sAddr);

    }

    void doByInterval()
    {
        regEvent(instanceIna3221(_addr)->getVoltage(INA3221_CH1), "Ina3221voltage1");
         
    }

    ~Ina3221voltage1(){};
};

class Ina3221curr1 : public IoTItem
{
private:
    uint8_t _addr = 0;

public:
    Ina3221curr1(String parameters) : IoTItem(parameters)
    {
        String sAddr;
        jsonRead(parameters, "addr", sAddr);
        if (sAddr == "")
            scanI2C();
        else
            _addr = hexStringToUint8(sAddr);
    }
    void doByInterval()
    {
        regEvent(instanceIna3221(_addr)->getCurrent(INA3221_CH1), "Ina3221curr1");
    }

    ~Ina3221curr1(){};
};

class Ina3221curr2 : public IoTItem
{
private:
    uint8_t _addr = 0;

public:
    Ina3221curr2(String parameters) : IoTItem(parameters)
    {
        String sAddr;
        jsonRead(parameters, "addr", sAddr);
        if (sAddr == "")
            scanI2C();
        else
            _addr = hexStringToUint8(sAddr);
    }
    void doByInterval()
    {
        regEvent(instanceIna3221(_addr)->getCurrent(INA3221_CH2), "Ina3221curr2");
    }

    ~Ina3221curr2(){};
};

class Ina3221curr3 : public IoTItem
{
private:
    uint8_t _addr = 0;

public:
    Ina3221curr3(String parameters) : IoTItem(parameters)
    {
        String sAddr;
        jsonRead(parameters, "addr", sAddr);
        if (sAddr == "")
            scanI2C();
        else
            _addr = hexStringToUint8(sAddr);
    }
    void doByInterval()
    {
        regEvent(instanceIna3221(_addr)->getCurrent(INA3221_CH3), "Ina3221curr3");
    }

    ~Ina3221curr3(){};
};


class Ina3221Setting : public IoTItem
{
private:
    uint8_t _addr = 0;
    int adjClbr = 0;
    int resol = 1;


public:
    Ina3221Setting(String parameters) : IoTItem(parameters)
    {
        String sAddr;
        jsonRead(parameters, "addr", sAddr);
        jsonRead(parameters, "adjClbr", adjClbr);
        jsonRead(parameters, "resol", resol);


        if (sAddr == "")
            scanI2C();
        else
            _addr = hexStringToUint8(sAddr);
            
        ina3221SettingArray[_addr] = new Ina3221Value;
        //jsonRead(parameters, "shunt", ina226SettingArray[_addr]->shunt);
        jsonRead(parameters, "maxV", ina3221SettingArray[_addr]->maxV);
        
       // instanceIna3221(_addr)->adjCalibration(adjClbr);

       // instanceIna3221(_addr)->setAveraging(resol);   // Напряжение в 12ти битном режиме
    }

    void onModuleOrder(String &key, String &value)
    {
        if (key == "getClbr")
        {
           // SerialPrint("i", F("Ina226"), "addr: " + String(_addr) + ", Value Calibration:" + instanceIna3221(_addr)->getCalibration());
        }
    }
/*
    IoTValue execute(String command, std::vector<IoTValue> &param)
    {
        if (command == "sleep")
        {
            if (param.size() == 1)
            {
                if (param[0].valD == 0)
                    instanceIna3221(_addr)->sleep(false);
                if (param[0].valD == 1)
                    instanceIna3221(_addr)->sleep(true);
                return {};
            }
        }


        return {};
    }
*/
    ~Ina3221Setting(){};
};

void *getAPI_Ina3221(String subtype, String param)
{
    if (subtype == F("Ina3221curr1"))
    {
        return new Ina3221curr1(param);
    }
    else if (subtype == F("Ina3221voltage1"))
    {
        return new Ina3221voltage1(param);
    }

    else
    {
        return nullptr;
    }
}
