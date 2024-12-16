#include "Global.h"
#include "classes/IoTItem.h"
#include <map>

#include <SPI.h>              //Library for using SPI Communication 
#include <mcp2515.h>          //Library for using CAN Communication (https://github.com/autowp/arduino-mcp2515/)
    
    
     struct can_frame canMsg;
     MCP2515 mcp2515(10);                 // SPI CS Pin 10

class CanBUSm : public IoTItem
{
private:


public:

  CanBUSm(String parameters) : IoTItem(parameters)
  {
      SPI.begin();                       //Begins SPI communication

           mcp2515.reset();
           mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ); //Sets CAN at speed 500KBPS and Clock 8MHz
           mcp2515.setNormalMode();                   //Sets CAN at normal mode

  }

  void doByInterval()
  
  {
    if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) // To receive data (Poll Read)
          {

             int tempInt = (canMsg.data[0] << 8) | canMsg.data[1]; // Объединить MSB и LSB
             float temperatureC = tempInt / 100.0; 

            value.valS = temperatureC;
              int x = canMsg.data[0];
           // int y = canMsg.data[1];

               regEvent(value.valS, "CanBUS"); 
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

