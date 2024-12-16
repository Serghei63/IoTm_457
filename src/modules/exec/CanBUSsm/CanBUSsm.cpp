#include "Global.h"
#include "classes/IoTItem.h"
#include <map>

//#include <SPI.h>              //Library for using SPI Communication 
//#include <mcp2515.h>          //Library for using CAN Communication (https://github.com/autowp/arduino-mcp2515/)
#include <CAN.h>


#define TX_GPIO_NUM   18  // Connects to CTX
#define RX_GPIO_NUM   19  // Connects to CRX

bool _debug;

class CanBUSsm : public IoTItem
{
private:


public:

  CanBUSsm(String parameters) : IoTItem(parameters)
  {

  Serial.println ("CAN Receiver/Receiver");

  // Set the pins
  CAN.setPins (RX_GPIO_NUM, TX_GPIO_NUM);

  // start the CAN bus at 500 kbps
  if (!CAN.begin (500E3)) {
    Serial.println ("Starting CAN failed!");
    while (1);
  }
  else {
    Serial.println ("CAN Initialized");
  }

  }

  void doByInterval()
  
  {
  // try to parse packet
  int packetSize = CAN.parsePacket();

  if (packetSize) {
    // received a packet
    Serial.print ("Received ");

    if (CAN.packetExtended()) {
      Serial.print ("extended ");
    }

    if (CAN.packetRtr()) {
      // Remote transmission request, packet contains no data
      Serial.print ("RTR ");
    }
    if (_debug)
    {
    Serial.print ("packet with id 0x");
    Serial.print (CAN.packetId(), HEX);
    }
    if (CAN.packetRtr()) {
      Serial.print (" and requested length ");
      Serial.println (CAN.packetDlc());
    } else {
          if (_debug)
    {
      Serial.print (" and length ");
      Serial.println (packetSize);
    }
      // only print packet data for non-RTR packets
      while (CAN.available()) {


        /*
             int tempInt = (canMsg.data[0] << 8) | canMsg.data[1]; // Объединить MSB и LSB
             float temperatureC = tempInt / 100.0; 

            value.valS = temperatureC;
              int x = canMsg.data[0];
           // int y = canMsg.data[1];

               regEvent(value.valS, "CanBUS");*/
       value.valS = (char) CAN.read();
       
       regEvent(value.valS, "CanBUSsm");

        Serial.print ((char) CAN.read());
      }
          if (_debug)
    {
      Serial.println();
    }
    }
    if (_debug)
    {
    Serial.println();
    }
  }
}

  ~CanBUSsm(){
    
  };

};

void *getAPI_CanBUSsm(String subtype, String param)
{

  if (subtype == F("canMaster"))
  {
    return new CanBUSsm(param);
  }

  {
    return nullptr;
  }
}

