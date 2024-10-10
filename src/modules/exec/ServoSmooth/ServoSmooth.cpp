
#include "Global.h"
#include "classes/IoTItem.h"
#include "classes/IoTGpio.h"

extern IoTGpio IoTgpio;                  // Подключаем библиотеку Wire

#include <ServoSmooth.h>
#include <Servo.h>
//#include <ESP32Servo.h>


class IoTServo : public IoTItem
{
private:
    String _addr;
    ServoSmooth *servo;
    int _oldValue;
    int _Value;
   

public:
    IoTServo(String parameters) : IoTItem(parameters)
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


        servo = new ServoSmooth(maxAngle);

        servo->attach(pin, minPulseWidth, maxPulseWidth);
        servo->setSpeed(speed);
        servo->setAccel(accel);

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
                regEvent(value.valD, "ServoSmooth");
            }
        }
        return {};
    }

    void setValue(const IoTValue &Value, bool genEvent = true)
    {
        value = Value;
        //if (value.isDecimal & (_oldValue != value.valD))
         if (value.isDecimal)
        {
           // _oldValue = value.valD;
          //  servo->setTargetDeg(_oldValue);
                        _Value = value.valD;
            servo->setTargetDeg(_Value);
            regEvent(value.valD, "ServoSmooth", false, genEvent);
        }
    }

    ~IoTServo()
    {
        delete servo;
    };
};

void *getAPI_ServoSmooth(String subtype, String param)
{
    if (subtype == F("ServoSmooth"))
    {
        return new IoTServo(param);
    }
    else
    {
        return nullptr;
    }
}