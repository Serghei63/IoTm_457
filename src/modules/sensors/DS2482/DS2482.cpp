/*
"https://github.com/Pfannex/DS18B20_DS2482",
"https://github.com/milesburton/Arduino-Temperature-Control-Library"

*/
#include "Global.h"
#include "classes/IoTItem.h"
#include <map>
/*
#include "DallasTemperature.h"
#include <OneWire.h>
#include <map>

//Tools
  #include <Wire.h>             //I²C

//DS2482
  #include <DS2482.h>
  #include <DS18B20_DS2482.h>


  DS2482 DS2482(0);
  DS18B20_DS2482 DS18B20_devices(&DS2482);
 // DS2482 DS2482(0);   // 0 = 0x18 Тут ругается на DS2482
 //DS18B20_DS2482 DS18B20_devices();
 // DS18B20_DS2482 DS18B20_devices(*DS2482);
  DeviceAddress DS18B20_Address;
  int DevicesCount = 0;
  int TemperatueCount = 0;
  

  unsigned long lastTemp = 0, tempScanInterval = 10 * 1000 * 1000;

*/

//i2cTemps - example for DS2482 library to use DS2482 operations:
//           - search the one-wire bus
//           - retrieve temperature values from DS18B20
//           - display temperatures
//
// started: Jan 21, 2022  G. D. (Joe) Young <jyoung@islandnet.com>
//
// revised: Jan 22/22 - add read scratchpad, calculate temperatures
//          Feb  1/22 - fix temperature calculation using printfix, add crc check 
//

#include <Wire.h>
#include "DS2482.h"   //package of AN3684 subr
#include "oneWire.h"  //DS18B20 definitions
#define PRSTRSIZE 5   //buffer size for printfix
#include "printfix.h" //package for printing fixed-width fields

#define I2Cadr 0x18   //base address of DS2482
#define MAXID 6       //maximum number of one-wire serial numbers

DS2482 i2ow( I2Cadr ); //create bridge object on I2C address 0x18

byte sna[MAXID][8];   //storage for discovered one-wire devices
int temps[MAXID];     //storage for returned temp integers

byte tmpMem[25];      //command string buffer
bool found = false;
float t = 0;
class DS2482t : public IoTItem {

    
   private:

   
    //для работы библиотеки с несколькими линиями  необходимо обеспечить каждый экземпляр класса ссылками на объекты настроенные на эти линии
  //  OneWire* _oneWire;


    
   // DallasTemperature* _sensor;
  //  DeviceAddress _deviceAddress;

      String _addri2c;
      String _addr;

   public:

   
    //=======================================================================================================
    // setup()
    // это аналог setup из arduino. Здесь вы можете выполнять методы инициализации сенсора.
    // Такие как ...begin и подставлять в них параметры полученные из web интерфейса.
    // Все параметры хранятся в перемененной parameters, вы можете прочитать любой параметр используя jsonRead функции:
    // jsonReadStr, jsonReadBool, jsonReadInt
    DS2482t(String parameters) : IoTItem(parameters) {


        Wire.begin( );
  i2ow.begin( );
  Serial.println( "owI2C - DS2482 bridge" );
  if( i2ow.DS2482_detect(  ) ) {
    Serial.println( "bridge chip detected and reset" );
  } else {
    Serial.print( "error accessing bridge chip at I2Cadr " );
    Serial.println( I2Cadr, HEX );
  }


    //set operating configuration
  if( i2ow.DS2482_write_config( bit(APU) ) ) {
    Serial.println( "configuration set" );
  } else {
    Serial.println( "failed to set config" );
  }

  //reset 1wire bus
  if( i2ow.OWReset( ) ) {
    Serial.println( "bus reset" );
  } else {
    Serial.println( "failed bus reset" );
  }
  if( i2ow.short_detected ) Serial.println( "  short detected" );
  Serial.println( "search ROM" );
  //search ROM with indefinite number of devices, up to MAXID
  byte jx = 0;
  if( i2ow.OWFirst( ) ) {
    found = true;
    for( byte ix=0; ix<8; ix++ ) sna[jx][ix] = i2ow.ROM_NO[ix];    //copy found to save area
    jx++;
    while( found && jx<MAXID ) {
      if( i2ow.OWNext( ) ) {
        for( byte ix=0; ix<8; ix++ ) sna[jx][ix] = i2ow.ROM_NO[ix];
        jx++;
      } else {
        found = false;
      }
    } // while finding devices (fewer than MAXID)
  } // if first
  if( jx == 0 ) {
    Serial.println( "no one-wire devices found" );
  } else {
    for( byte ix=0; ix<jx; ix++ ) {
      for( byte kx=0; kx<8; kx++ ) {
        Serial.print( ' ' );
        Serial.print( sna[ix][kx], HEX );    
      }
      Serial.println( "" );
    }
  }

  if( jx != 0 ) {
    getTemp( jx );
    //calculate decimal values - temps are 16X actual value
    Serial.println( "temperatures for each sensor:" );
    for( byte ix=0; ix<jx; ix++ ) {
      int itemp = temps[ix];
      if( itemp < 0 ) {
        itemp = -itemp;
        Serial.print( '-' );
      }
      Serial.print( itemp>>4 );
      Serial.print( '.' );
      printFixUint( ( (itemp & 0x0f) * 1000 )/16, 3, prStr );
      Serial.print( prStr );
      Serial.print( ' ' );
    }
    Serial.println( "" );
  } //if devices found
/*
    // DS2482 DS2482(0);   // 0 = 0x18
   //  DS18B20_DS2482 DS18B20_devices(&DS2482); 

  Serial.println("\n\nDS2482-100 Test\n\n");
  delay(1000);

  Serial.print("starting I2C: ");
 // Wire.begin(SDA, SCL);           //Start I²C
  Wire.begin();           //Start I²C
  Serial.println("ok");
  scanI2C();
   

  Serial.print("DS2482-100 reset: ");
  DS2482.reset();
  Serial.println("ok");

//search for devices and print address = true  
  Serial.print("DS2482-100 scan: \n");
  DevicesCount = DS2482.devicesCount(true);
  Serial.println("devices: #" + (String)DevicesCount);

  delay(1000);
  Serial.print("DS18B20 begin: ");
  DS18B20_devices.begin();
  Serial.println("ok");
  TemperatueCount = DS18B20_devices.getDeviceCount();



  Serial.println("devices: #" + (String)TemperatueCount);
  return;

  
*/
        int index;
      
        jsonRead(parameters, "index", index, false);
        jsonRead(parameters, "addri2c", _addri2c, false);
        jsonRead(parameters, "addr", _addr, false);

      //if (_addri2c == "") {
      //      scanI2C();
      //      return;
      //  }
/*
        //учитываем, что библиотека может работать с несколькими линиями на разных пинах, поэтому инициируем библиотеку, если линия ранее не использовалась
        if (oneWireTemperatureArray.find(pin) == oneWireTemperatureArray.end()) {
            _oneWire = new OneWire((uint8_t)pin);
            oneWireTemperatureArray[pin] = _oneWire;
        } else {
            _oneWire = oneWireTemperatureArray[pin];
        }
*/


// Это не работает

 //       _sensor = new DallasTemperature(_oneWire);
  //    _sensor->begin();
      // _sensors->setResolution(12);

        //Определяем адрес. Если параметр addr не установлен, то узнаем адрес по индексу
     //   if (_addr == "") {
      //      _sensor->getAddress(_deviceAddress, index);
      //      char addrStr[20] = "";
      //      hex2string(_deviceAddress, 8, addrStr);
    //        SerialPrint("I", "Sensor " + (String)_id, "index: " + (String)index + " addr: " + String(addrStr));
     //   } else {
     //       string2hex(_addr.c_str(), _deviceAddress);
     //   }
        
    }

    
    //=======================================================================================================
    // doByInterval()
    // это аналог loop из arduino, но вызываемый каждые int секунд, заданные в настройках. Здесь вы должны выполнить чтение вашего сенсора
    // а затем выполнить regEvent - это регистрация произошедшего события чтения
    // здесь так же доступны все переменные из секции переменных, и полученные в setup
    // если у сенсора несколько величин то делайте несколько regEvent
    // не используйте delay - помните, что данный loop общий для всех модулей. Если у вас планируется длительная операция, постарайтесь разбить ее на порции
    // и выполнить за несколько тактов
    void doByInterval() {

      int kkx = 0;
      int itemp = 0;
      
    //  value.valD = emon1.calcIrms(1480);  // Calculate Irms only       
     // regEvent(value.valD, "current");
t = printFixUint;
        //запускаем опрос измерений у всех датчиков на линии

     // value.valD = DS18B20_devices.requestTemperaturesByIndex(0);
      // Serial.print(DS18B20_devices.requestTemperaturesByIndex(0));
     // SerialPrint("E", "Sensor Ds18b20", "Error", DS18B20_devices.requestTemperaturesByIndex(0));
     //   DS18B20_devices.requestTemperatures();
//current_temp[0] = DS18B20_devices.getTempC(tempSensor[0]);
        //получаем температуру по адресу
        value.valD = t;

      // if (value.valD != DEVICE_DISCONNECTED_C)
            regEvent(value.valD, "");  //обязательный вызов для отправки результата работы
        //    regEvent(value.valD, "Ds2482");  //обязательный вызов для отправки результата работы
       // else
          // SerialPrint("E", "Sensor Ds18b20", "Error", _id);
            

/*
    unsigned long currMicros = micros();
  unsigned long nextTemp;

  nextTemp = lastTemp + tempScanInterval;
  delay(100);
  
  // temperature
  if (nextTemp < currMicros) {
    for (uint8_t i = 0; i < TemperatueCount; i++){
     DS18B20_devices.requestTemperaturesByIndex(i);
       regEvent(DS18B20_devices.requestTemperaturesByIndex(0), "Ds2482");  //обязательный вызов для отправки результата работы
      Serial.print("temp #");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(DS18B20_devices.getTempCByIndex(i));
    }

    lastTemp = currMicros;
  }
  */
    }

    void getTemp( byte numTemp ) {
  int kkx = 0;

  //meas with power
  Serial.println( "measure with power" );
  i2ow.OWReset( );
  i2ow.OWWriteByte( CSKRM );     //skip ROM
  i2ow.OWWriteBytePower( CMD_WWBP ); //strong pullup, start conversion
  delay(1000);
  i2ow.OWLevel( MODE_STANDARD );    //restore normal pullup

  for( byte kx=0; kx<numTemp ; kx++ ) {
    kkx = kx;
    i2ow.OWReset( );
    tmpMem[0] = CMTCH;
    for( byte ix=0; ix<8; ix++ ) tmpMem[ix+1] = sna[kx][ix];
    tmpMem[9] = CRSPD;
    for( byte ix=0; ix<9; ix++ ) tmpMem[ix+10] = 0xFF;
    i2ow.OWBlock( tmpMem, 19 );
    uint8_t crc8 = 0;
    for( byte ix=0; ix<9; ix++ ) {
      crc8 = i2ow.calc_crc8( tmpMem[ix+10] );       //accumulate crc for scratchpad bytes
      Serial.print( ' ' );
      Serial.print( tmpMem[ix+10], HEX );
    }
    Serial.print( ' ' );
    Serial.println( crc8 );                      //report crc (should be zero)
    temps[kkx] = ((int)(tmpMem[11])<<8) + (int)tmpMem[10];   //save raw temp value
  } //loop to report each found device

}
    //=======================================================================================================

    ~DS2482t() {
        //if (_sensor) delete _sensor;
    };
};

// после замены названия сенсора, на функцию можно не обращать внимания
// если сенсор предполагает использование общего объекта библиотеки для нескольких экземпляров сенсора, то в данной функции необходимо предусмотреть
// создание и контроль соответствующих глобальных переменных
void* getAPI_DS2482(String subtype, String param) {
    if (subtype == F("Ds2482t")) {
        return new DS2482t(param);
    } else {
        return nullptr;
    }
}
