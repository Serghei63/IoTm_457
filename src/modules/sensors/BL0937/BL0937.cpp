
#include "Global.h"
#include "classes/IoTItem.h"

#include "BL0937lib.h"
// #include "classes/IoTUart.h"
// #include <map>
BL0937 *bl0937 = nullptr;

class BL0937v : public IoTItem
{
private:
public:
    BL0937v(String parameters) : IoTItem(parameters)
    {
    }

    void doByInterval()
    {
        if (bl0937)
            regEvent(bl0937->getVoltage(), "BL0937 V");
        else
        {
            regEvent(NAN, "BL0937v");
            SerialPrint("E", "BL0937cmd", "initialization error", _id);
        }
    }

    ~BL0937v(){};
};

class BL0937a : public IoTItem
{
private:
public:
    BL0937a(String parameters) : IoTItem(parameters)
    {
    }

    void doByInterval()
    {
        if (bl0937)
            regEvent(bl0937->getCurrent(), "BL0937 A");
        else
        {
            regEvent(NAN, "BL0937a");
            SerialPrint("E", "BL0937cmd", "initialization error", _id);
        }
    }

    ~BL0937a(){};
};

class BL0937w : public IoTItem
{
private:
public:
    BL0937w(String parameters) : IoTItem(parameters)
    {
    }

    void doByInterval()
    {
        if (bl0937)
            regEvent(bl0937->getApparentPower(), "BL0937 W");
        else
        {
            regEvent(NAN, "BL0937w");
            SerialPrint("E", "BL0937cmd", "initialization error", _id);
        }
    }

    ~BL0937w(){};
};

class BL0937reactw : public IoTItem
{
private:
public:
    BL0937reactw(String parameters) : IoTItem(parameters)
    {
    }

    void doByInterval()
    {
        if (bl0937)
            regEvent(bl0937->getReactivePower(), "BL0937 reactW");
        else
        {
            regEvent(NAN, "BL0937reactw");
            SerialPrint("E", "BL0937cmd", "initialization error", _id);
        }
    }

    ~BL0937reactw(){};
};

class BL0937actw : public IoTItem
{
private:
public:
    BL0937actw(String parameters) : IoTItem(parameters)
    {
    }

    void doByInterval()
    {
        if (bl0937)
            regEvent(bl0937->getActivePower(), "BL0937 actW");
        else
        {
            regEvent(NAN, "BL0937actw");
            SerialPrint("E", "BL0937cmd", "initialization error", _id);
        }
    }

    ~BL0937actw(){};
};

class BL0937wh : public IoTItem
{
private:
public:
    BL0937wh(String parameters) : IoTItem(parameters)
    {
    }

    void doByInterval()
    {
        if (bl0937)
            regEvent(bl0937->getEnergy() / 3600.0 / 1000.0, "BL0937 Wh");
        else
        {
            regEvent(NAN, "BL0937wh");
            SerialPrint("E", "BL0937cmd", "initialization error", _id);
        }
    }

    ~BL0937wh(){};
};

void ICACHE_RAM_ATTR bl0937_cf1_interrupt()
{
    bl0937->cf1_interrupt();
}
void ICACHE_RAM_ATTR bl0937_cf_interrupt()
{
    bl0937->cf_interrupt();
}

class BL0937cmd : public IoTItem
{
private:
    float CURRENT_RESISTOR = 0.001;          // Нужна возможность задавать из веб, это по умолчанию
    int VOLTAGE_RESISTOR_UPSTREAM = 1000000; // Нужна возможность задавать  из веб, это по умолчанию
    int VOLTAGE_RESISTOR_DOWNSTREAM = 1000;  // Нужна возможность задавать из веб, это по умолчанию
    int BL0937_CF_GPIO = 4;                  // 8266 12                            //Нужна возможность задавать пин из веб, это по умолчанию
    int BL0937_CF1_GPIO = 5;                 // 8266 13                            //Нужна возможность задавать пин из веб, это по умолчанию
    int BL0937_SEL_GPIO_INV = 12;            // 8266 15 // inverted    //Нужна возможность задавать пин из веб, это по умолчанию
    float _expV = 0;
    float _expA = 0;
    float _expW = 0;

public:
    BL0937cmd(String parameters) : IoTItem(parameters)
    {
        jsonRead(parameters, "R_current", CURRENT_RESISTOR);
        jsonRead(parameters, "R_upstream", VOLTAGE_RESISTOR_UPSTREAM);
        jsonRead(parameters, "R_downstream", VOLTAGE_RESISTOR_DOWNSTREAM);
        jsonRead(parameters, "CF_GPIO", BL0937_CF_GPIO);
        jsonRead(parameters, "CF1_GPIO", BL0937_CF1_GPIO);
        jsonRead(parameters, "SEL_GPIO", BL0937_SEL_GPIO_INV);
        jsonRead(parameters, "expV", _expV);
        jsonRead(parameters, "expA", _expA);
        jsonRead(parameters, "expW", _expW);
        bl0937 = new BL0937;
        bl0937->begin(BL0937_CF_GPIO, BL0937_CF1_GPIO, BL0937_SEL_GPIO_INV, LOW, true);
        bl0937->setResistors(CURRENT_RESISTOR, VOLTAGE_RESISTOR_UPSTREAM, VOLTAGE_RESISTOR_DOWNSTREAM);
        attachInterrupt(BL0937_CF1_GPIO, bl0937_cf1_interrupt, FALLING);
        attachInterrupt(BL0937_CF_GPIO, bl0937_cf_interrupt, FALLING);
        if (_expV)
            bl0937->expectedVoltage(_expV); // для калибровки вольтаж нужно вводить из веб интерфейса
        if (_expV)
            bl0937->expectedCurrent(_expA); // для калибровки можно так, а лучше ток вводить из веб интерфейса
        if (_expV)
            bl0937->expectedActivePower(_expW); // для калибровки потребляемую мощность нужно вводить из веб интерфейса
    }

    void doByInterval()
    {
    }

    void onModuleOrder(String &key, String &value)
    {
        if (bl0937)
        {
            if (key == "reset")
            {
                bl0937->resetEnergy();
                SerialPrint("i", "BL0937", "reset energy done");
            }
        }
    }
    /*
        IoTValue execute(String command, std::vector<IoTValue> &param)
        {
            if (!bl0937)
                return {};
            if (command == "calibration")
            {
                if (param.size() == 3)
                {
                    float v = param[0].valD;
                    float a = param[1].valD;
                    float p = param[2].valD;
                    bl0937->expectedVoltage(v);     // для калибровки вольтаж нужно вводить из веб интерфейса
                    bl0937->expectedCurrent(a);     // для калибровки можно так, а лучше ток вводить из веб интерфейса
                    bl0937->expectedActivePower(p); // для калибровки потребляемую мощность нужно вводить из веб интерфейса
                    return {};
                }
            }
            return {};
        }
    */
    ~BL0937cmd()
    {
        if (bl0937)
        {
            delete bl0937;
            bl0937 = nullptr;
        }
    };
};

void *getAPI_BL0937(String subtype, String param)
{
    if (subtype == F("BL0937v"))
    {
        return new BL0937v(param);
    }
    else if (subtype == F("BL0937a"))
    {
        return new BL0937a(param);
    }
    else if (subtype == F("BL0937w"))
    {
        return new BL0937w(param);
    }
    else if (subtype == F("BL0937wh"))
    {
        return new BL0937wh(param);
    }
    else if (subtype == F("BL0937reactw"))
    {
        return new BL0937reactw(param);
    }
    else if (subtype == F("BL0937actw"))
    {
        return new BL0937actw(param);
    }
    else if (subtype == F("BL0937cmd"))
    {
        return new BL0937cmd(param);
    }
    else
    {
        return nullptr;
    }
}
