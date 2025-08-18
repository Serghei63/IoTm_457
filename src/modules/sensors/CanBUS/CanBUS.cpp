#include "Global.h"
#include "classes/IoTItem.h"
#include <map>

#include <SPI.h>              //Library for using SPI Communication 
#include <mcp2515.h>          //Library for using CAN Communication (https://github.com/autowp/arduino-mcp2515/)
 

extern IoTGpio IoTgpio;
    
     struct can_frame canMsg;
     MCP2515 mcp2515(5);                 // SPI CS Pin 5

bool _debug;                         

class CanBUSm : public IoTItem
{
private:
 String _regStr = "";  // Адрес регистра который будем дергать ( по коду от 0х0000 до 0х????)
 String _regAsk = "";  // Адрес регистра который будем дергать ( по коду от 0х0000 до 0х????)
 String _dlcStr = "";  // длинна данных (Data Length Code)
 uint16_t _reg = 0;
 uint16_t _ask = 0;
 uint8_t _addr = 0;    // Адрес слейва от 1 до 247
 uint8_t _dlc = 0;     // Длина данных (Data Length Code)

public:

  CanBUSm(String parameters) : IoTItem(parameters)
  {
    // адреса регистров прочитаем с веба
      _addr = jsonReadInt(parameters, "addr"); 
      jsonRead(parameters, "can_id", _regStr);    // адреса регистров прочитаем с веба
      _reg = hexStringToUint16(_regStr);
      jsonRead(parameters, "ask_id", _regAsk);    // адреса регистров прочитаем с веба
      _ask = hexStringToUint16(_regAsk);
      _dlc = jsonReadInt(parameters, "dlc", 8); // Длина данных (Data Length Code) по умолчанию 8
      jsonRead(parameters, "debug", _debug);
      SPI.begin();                                                 //Begins SPI communication

           mcp2515.reset();
           mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);              //Sets CAN at speed 500KBPS and Clock 8MHz
           mcp2515.setNormalMode();                                //Sets CAN at normal mode

  }

  

  void doByInterval()
  
  {
    if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK)         // To receive data (Poll Read)
        {

             if (canMsg.can_id == _reg)                           // Проверьте, пришло ли сообщение от отправителя
          {

             int tempInt = (canMsg.data[_addr]);
        
             value.valS = tempInt;
              int x = canMsg.data[_addr];
            
        if (_debug)
      {
                        Serial.print("Data received: ");
                        Serial.print(tempInt);
                     // Serial.println(" °C");
      }       

               regEvent(value.valS, "CanBUS");
    
                 //Отправить подтверждение
                 
                 canMsg.can_id  = _ask;                              // Использовать идентификатор подтверждения
                 canMsg.can_dlc = 0;                                // Никакие данные не нужны для ACK   // Задаем размер кадра
                 mcp2515.sendMessage(&canMsg);
        if (_debug)
      {          
                 Serial.println("ACK sent"); 
      }
    }
   }

  }

  ~CanBUSm(){
    
  };

};

void *getAPI_CanBUS(String subtype, String param)
{

  if (subtype == F("canMaster"))
  {
    return new CanBUSm(param);
  }

  {
    return nullptr;
  }
}

