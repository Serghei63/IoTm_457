#include "Global.h"
#include "classes/IoTItem.h"
#include <map>
#include <HardwareSerial.h>

#include "rsEctoControl.h"

// class ModbusUart;
Stream *_mbUART = nullptr;

#define UART_LINE 2

// Modbus stuff
// Данные Modbus по умолчанию

#define MODBUS_RX_PIN 18        // Rx pin
#define MODBUS_TX_PIN 19        // Tx pin
#define MODBUS_SERIAL_BAUD 9600 // Baud rate for esp32 and max485 communication

// void modbusPreTransmission()
// {
//   //  delay(500);
//   if (_DIR_PIN)
//     digitalWrite(_DIR_PIN, HIGH);
// }

// // Pin 4 made low for Modbus receive mode
// // Контакт 4 установлен на низком уровне для режима приема Modbus
// void modbusPostTransmission()
// {
//   if (_DIR_PIN)
//     digitalWrite(_DIR_PIN, LOW);
//   //  delay(500);
// }

// ModbusMaster node;

RsEctoControl *rsEC;

class EctoControlAdapter : public IoTItem
{
private:
  int _rx = MODBUS_RX_PIN; // адреса прочитаем с веба
  int _tx = MODBUS_TX_PIN;
  int _baud = MODBUS_SERIAL_BAUD;
  String _prot = "SERIAL_8N1";
  int protocol = SERIAL_8N1;
  uint8_t _addr = 0xF0; // Адрес слейва от 1 до 247
  uint8_t _type = 0x14; // Тип устройства: 0x14 – адаптер OpenTherm (вторая версия); 0x11 – адаптер OpenTherm (первая версия, снята с производства)
  bool _debugLevel;     // Дебаг
  //uint8_t _dir_pin;

public:
  EctoControlAdapter(String parameters) : IoTItem(parameters)
  {
    //_dir_pin = 0;
    _addr = jsonReadInt(parameters, "addr"); // адреса slave прочитаем с веба
    _rx = jsonReadInt(parameters, "RX");     // прочитаем с веба
    _tx = jsonReadInt(parameters, "TX");
    //_dir_pin = jsonReadInt(parameters, "DIR_PIN");
    _baud = jsonReadInt(parameters, "baud");
    _prot = jsonReadStr(parameters, "protocol");
    jsonRead(parameters, "debug", _debugLevel);

    if (_prot == "SERIAL_8N1")
    {
      protocol = SERIAL_8N1;
    }
    else if (_prot == "SERIAL_8N2")
    {
      protocol = SERIAL_8N2;
    }

    // Serial2.begin(baud-rate, protocol, RX pin, TX pin);

    _mbUART = new HardwareSerial(UART_LINE);

    if (_debugLevel > 2)
    {
      SerialPrint("I", "EctoControlAdapter", "baud: " + String(_baud) + ", protocol: " + String(protocol, HEX) + ", RX: " + String(_rx) + ", TX: " + String(_tx));
    }
    ((HardwareSerial *)_mbUART)->begin(_baud, protocol, _rx, _tx); // выбираем тип протокола, скорость и все пины с веба
    ((HardwareSerial *)_mbUART)->setTimeout(200);
    rsEC = new RsEctoControl(parameters);
    rsEC->begin(_addr, (Stream &)*_mbUART);
  }

  void doByInterval()
  {
    if (rsEC)
      rsEC->doByInterval();
  }

  void loop()
  {
    if (rsEC)
      rsEC->loop(isNetworkActive(), mqttIsConnect());
    // для новых версий IoTManager
    IoTItem::loop();
  }

  IoTValue execute(String command, std::vector<IoTValue> &param)
  {
    if (rsEC)
      rsEC->execute(command, param);
    else
      return {};
    if (command == "getModelVersion")
    {
      rsEC->getModelVersion();
    }
    if (command == "getModelVersion")
    {
      rsEC->getModelVersion();
    }
    if (command == "getBoilerInfo")
    {
      rsEC->getBoilerInfo();
    }
    if (command == "getBoilerStatus")
    {
      rsEC->getBoilerStatus();
    }
    if (command == "getCodeError")
    {
      rsEC->getCodeError();
    }
    if (command == "getCodeErrorExt")
    {
      rsEC->getCodeErrorExt();
    }
    if (command == "getFlagErrorOT")
    {
      rsEC->getFlagErrorOT();
    }
    if (command == "getFlowRate")
    {
      rsEC->getFlowRate();
    }
    if (command == "getMaxSetCH")
    {
      rsEC->getMaxSetCH();
    }
    if (command == "getMaxSetDHW")
    {
      rsEC->getMaxSetDHW();
    }
    if (command == "getMinSetCH")
    {
      rsEC->getMinSetCH();
    }
    if (command == "getMinSetDHW")
    {
      rsEC->getMinSetDHW();
    }
    if (command == "getModLevel")
    {
      rsEC->getModLevel();
    }
    if (command == "getPressure")
    {
      rsEC->getPressure();
    }
    if (command == "getTempCH")
    {
      rsEC->getTempCH();
    }
    if (command == "getTempDHW")
    {
      rsEC->getTempDHW();
    }
    if (command == "getTempOutside")
    {
      rsEC->getTempOutside();
    }

    if (command == "setTypeConnect")
    {
      rsEC->setTypeConnect(param[0].valD);
    }
    if (command == "setTCH")
    {
      rsEC->setTCH(param[0].valD);
    }
    if (command == "setTCHFaultConn")
    {
      rsEC->setTCHFaultConn(param[0].valD);
    }
    if (command == "setMinCH")
    {
      rsEC->setMinCH(param[0].valD);
    }
    if (command == "setMaxCH")
    {
      rsEC->setMaxCH(param[0].valD);
    }
    if (command == "setMinDHW")
    {
      rsEC->setMinDHW(param[0].valD);
    }
    if (command == "setMaxDHW")
    {
      rsEC->setMaxDHW(param[0].valD);
    }
    if (command == "setTDHW")
    {
      rsEC->setTDHW(param[0].valD);
    }
    if (command == "setMaxModLevel")
    {
      rsEC->setMaxModLevel(param[0].valD);
    }
    if (command == "setStatusCH")
    {
      rsEC->setStatusCH((bool)param[0].valD);
    }
    if (command == "setStatusDHW")
    {
      rsEC->setStatusDHW((bool)param[0].valD);
    }
    if (command == "setStatusCH2")
    {
      rsEC->setStatusCH2((bool)param[0].valD);
    }

    if (command == "lockOutReset")
    {
      rsEC->lockOutReset();
    }
    if (command == "rebootAdapter")
    {
      rsEC->rebootAdapter();
    }
    return {};
  }

  void publishData(String widget, String status)
  {

    IoTItem *tmp = findIoTItem(widget);
    if (tmp)
      tmp->setValue(status, true);
    else
    {
      if (_debugLevel > 0)
        SerialPrint("i", "NEW", widget + " = " + status);
    }
  }

  static void sendTelegramm(String msg)
  {
    if (tlgrmItem)
      tlgrmItem->sendTelegramMsg(false, msg);
  }

  ~EctoControlAdapter()
  {
    delete rsEC;
    rsEC = nullptr;
  };
};

void *getAPI_EctoControlAdapter(String subtype, String param)
{

  if (subtype == F("ecAdapter"))
  {
    return new EctoControlAdapter(param);
  }
  {
    return nullptr;
  }
}
