
#include "Global.h"
#include "classes/IoTItem.h"

// Подключаем библиотеки

#ifdef ESP32
#include <ESP32Servo.h>
#endif

#ifdef ESP8266
#include <Servo.h>
#endif

#include <ServoDriverSmooth.h>

// ServoDriverSmooth servo(0x40);      // с указанием адреса драйвера
// ServoDriverSmooth servo(0x40, 270); // с указанием адреса и макс. угла

class ServoPCA9685 : public IoTItem
{
private:
    String _addr;
    int _oldValue;
    ServoDriverSmooth *servo;
    // Pca9685Driver* _driver;

public:
    ServoPCA9685(String parameters) : IoTItem(parameters)
    {

        int pin, minPulseWidth, maxPulseWidth, speed, maxAngle;
        float accel;
        jsonRead(parameters, "pin", pin);
        jsonRead(parameters, "minPulseWidth", minPulseWidth);
        jsonRead(parameters, "maxPulseWidth", maxPulseWidth);
        // jsonRead(parameters, "minAngle", minAngle);
        jsonRead(parameters, "maxAngle", maxAngle);
        jsonRead(parameters, "speed", speed);
        jsonRead(parameters, "accel", accel);

        jsonRead(parameters, "addr", _addr);
        if (_addr == "")
        {
            scanI2C();
            return;
        }

        //servo = new ServoDriverSmooth(_addr, (int)maxAngle);// Тут обругало
        servo = new ServoDriverSmooth();

        servo->attach(pin, minPulseWidth, maxPulseWidth);
        servo->setSpeed(speed);
        servo->setAccel(accel);

        // _driver = new Pca9685Driver(index, _pwm);
    }

    void loop()
    {
        servo->tick();
        IoTItem::loop();
    }

    IoTValue execute(String command, std::vector<IoTValue> &param)
    {
        if (command == "rotate")
        {
            if (param.size())
            {
                value.valD = param[0].valD;
                servo->setTargetDeg(value.valD);
                regEvent(value.valD, "ServoPCA9685");
            }
        }
        return {};
    }

    void setValue(const IoTValue &Value, bool genEvent = true)
    {
        value = Value;
        if (value.isDecimal & (_oldValue != value.valD))
        {
            _oldValue = value.valD;
            servo->setTargetDeg(_oldValue);
            regEvent(value.valD, "ServoPCA9685", false, genEvent);
        }
    }

    ~ServoPCA9685()
    {
        delete servo;
    };
};

void *getAPI_ServoPCA9685(String subtype, String param)
{
    if (subtype == F("ServoPCA9685"))
    {
        return new ServoPCA9685(param);
    }
    else
    {
        return nullptr;
    }
}