#include "Global.h"
#include "classes/IoTItem.h"

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

// An IR detector/demodulator is connected to GPIO pin 14(D5 on a NodeMCU
// board).
// Note: GPIO 16 won't work on the ESP8266 as it does not have interrupts.
// Note: GPIO 14 won't work on the ESP32-C3 as it causes the board to reboot.
#ifdef ARDUINO_ESP32C3_DEV
const uint16_t kRecvPin = 10;  // 14 on a ESP32-C3 causes a boot loop.
#else  // ARDUINO_ESP32C3_DEV
const uint16_t kRecvPin = 14;
#endif  // ARDUINO_ESP32C3_DEV

IRrecv irrecv(kRecvPin);

decode_results results;

class IRswitch : public IoTItem {
   private:
    int _pinRx; // Выход радио модуля
    int _pinTx; // Выход модуля передатчика

    //

   public:
    IRswitch(String parameters): IoTItem(parameters) {
        jsonRead(parameters, "pinRx", _pinRx);// приемник
        jsonRead(parameters, "pinTx", _pinTx);//передатчик
		_interval = _interval / 1000;   // корректируем величину интервала int, теперь он в миллисекундах
		
        if (_pinRx >= 0)

            irrecv.enableIRIn();  // Start the receiver

    }

    void doByInterval() {

         if (irrecv.decode(&results))  {

          value.valD = (results.value, HEX);
                
            regEvent(value.valD, "IRswitch");         //обязательный вызов хотяб один раз 

      }

 irrecv.resume();  // Receive the next value
    }

    ~IRswitch() {};
};

void* getAPI_IRswitch(String subtype, String param) {
    if (subtype == F("IRswitch")) {
        return new IRswitch(param);
    } else {
        return nullptr;  
    }
}
