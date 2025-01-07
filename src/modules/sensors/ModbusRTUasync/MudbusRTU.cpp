#include "Global.h"
#include "classes/IoTItem.h"
#include <map>
#include <HardwareSerial.h>

#include "Logging.h"
#include "ModbusClientRTU.h"
#include "CoilData.h"

// class ModbusUart;
Stream *_modbusUART = nullptr;

// Данные Modbus по умолчанию
int8_t _DIR_PIN = 0;
#define UART_LINE 2
#define MODBUS_RX_PIN 18        // Rx pin
#define MODBUS_TX_PIN 19        // Tx pin
#define MODBUS_SERIAL_BAUD 9600 // Baud rate for esp32 and max485 communication

bool data_ready = false;
uint32_t request_time;

ModbusClientRTU *MB = nullptr;

ModbusClientRTU *instanceModBus(int8_t _DR)
{
  if (!MB)
  { // Если библиотека ранее инициализировалась, т о просто вернем указатель
    // Инициализируем библиотеку
    if (_DR)
      MB = new ModbusClientRTU(_DR);
    else
      MB = new ModbusClientRTU();
  }
  return MB;
}
// ModbusClientRTU MB(_DIR_PIN);
// ModbusClientRTU MB();

// TODO УБРАТЬ ЭТО ИЗ ПРИМЕРА
#define NUM_VALUES 21
float values[NUM_VALUES];
// Define an onData handler function to receive the regular responses
// Arguments are received response message and the request's token
void handleModBusData(ModbusMessage response, uint32_t token)
{
  printf("Response --- FC:%02X Server:%d Length:%d\n",
         response.getFunctionCode(),
         response.getServerID(),
         response.size());
  HEXDUMP_N("Data dump", response.data(), response.size());
  data_ready = true;
}

// Define an onError handler function to receive error responses
// Arguments are the error code returned and a user-supplied token to identify the causing request
void handleModBusError(Error error, uint32_t token)
{
  // ModbusError wraps the error code and provides a readable error message for it
  ModbusError me(error);
  // LOG_E("Error response: %02X - %s\n", (int)me, (const char *)me);
  Serial.printf("Error response: %02X - %s\n", (int)me, (const char *)me);
}

class ModbusClientAsync : public IoTItem
{
private:
  int8_t _rx = MODBUS_RX_PIN; // адреса прочитаем с веба
  int8_t _tx = MODBUS_TX_PIN;
  int _baud = MODBUS_SERIAL_BAUD;
  String _prot = "SERIAL_8N1";
  int protocol = SERIAL_8N1;

  int _addr = 0;       // Адрес слейва от 1 до 247 ( вроде )
  String _regStr = ""; // Адрес регистра который будем дергать ( по коду от 0х0000 до 0х????)
  uint16_t _reg = 0;
  bool _debug; // Дебаг

public:
  ModbusClientAsync(String parameters) : IoTItem(parameters)
  {
    _rx = (int8_t)jsonReadInt(parameters, "RX"); // прочитаем с веба
    _tx = (int8_t)jsonReadInt(parameters, "TX");
    _DIR_PIN = (int8_t)jsonReadInt(parameters, "DIR_PIN");
    _baud = jsonReadInt(parameters, "baud");
    _prot = jsonReadStr(parameters, "protocol");
    jsonRead(parameters, "debug", _debug);

    if (_prot == "SERIAL_8N1")
    {
      protocol = SERIAL_8N1;
    }
    else if (_prot == "SERIAL_8N2")
    {
      protocol = SERIAL_8N2;
    }

    pinMode(_DIR_PIN, OUTPUT);
    digitalWrite(_DIR_PIN, LOW);

    // Serial2.begin(baud-rate, protocol, RX pin, TX pin);
    instanceModBus(_DIR_PIN);
    _modbusUART = new HardwareSerial(UART_LINE);

    if (_debug)
    {
      SerialPrint("I", "ModbusClientAsync", "baud: " + String(_baud) + ", protocol: " + String(protocol, HEX) + ", RX: " + String(_rx) + ", TX: " + String(_tx));
    }
    RTUutils::prepareHardwareSerial((HardwareSerial &)*_modbusUART);
    // Serial2.begin(BAUDRATE, SERIAL_8N1, RXPIN, TXPIN);
    ((HardwareSerial *)_modbusUART)->begin(_baud, protocol, _rx, _tx); // выбираем тип протокола, скорость и все пины с веба
    ((HardwareSerial *)_modbusUART)->setTimeout(200);

    // Set up ModbusRTU client.
    // - provide onData handler function
    MB->onDataHandler(&handleModBusData);
    // - provide onError handler function
    MB->onErrorHandler(&handleModBusError);
    // Set message timeout to 2000ms
    MB->setTimeout(2000);
    // Start ModbusRTU background task
    MB->begin((HardwareSerial &)*_modbusUART);
  }

  // Комманды из сценария
  IoTValue execute(String command, std::vector<IoTValue> &param)
  {
    IoTValue val;
    uint8_t result;
    uint32_t reading;

    uint16_t _reg = 0;
    uint8_t count = 1;
    bool isFloat = 0;
    if (command == "readInputRegisters") // vout = mb.readInputRegisters(1, "0х0000", 1, 0) - "Адрес","Регистр","Кличество регистров","1-float, 0-long"
    {
      if (param.size())
      {
        _addr = param[0].valD;
        _reg = hexStringToUint16(param[1].valS);
        count = (uint8_t)param[2].valD;
        count = count > 2 ? 2 : count;
        count = count < 1 ? 1 : count;
        isFloat = (bool)param[3].valD;
        // val.valD = readFunctionModBus(0x04, _addr, _reg, count, isFloat);
        uint32_t lastMillis = millis();
        Serial.printf("sending request with token %d\n", (uint32_t)lastMillis);
        Error err;
        err = MB->addRequest((uint32_t)lastMillis, _addr, READ_INPUT_REGISTER, _reg, count);
        if (err != SUCCESS)
        {
          ModbusError e(err);
          Serial.printf("Error creating request: %02X - %s\n", (int)e, (const char *)e);
        }
      }
      return val;
    }
    else if (command == "readHoldingRegisters") // vout = mb.readHoldingRegisters(1, "0х0000", 2, 1) - "Адрес","Регистр","Кличество регистров","1-float, 0-long"
    {
      if (param.size())
      {
        _addr = param[0].valD;
        _reg = hexStringToUint16(param[1].valS);
        count = (uint8_t)param[2].valD;
        count = count > 2 ? 2 : count;
        count = count < 1 ? 1 : count;
        isFloat = (bool)param[3].valD;
        // val.valD = readFunctionModBus(0x03, _addr, _reg, count, isFloat);

        // Create request for
        // (Fill in your data here!)
        // - server ID = 1
        // - function code = 0x03 (read holding register)
        // - start address to read = word 10
        // - number of words to read = 4
        // - token to match the response with the request. We take the current millis() value for it.
        //
        // If something is missing or wrong with the call parameters, we will immediately get an error code
        // and the request will not be issued
        uint32_t lastMillis = millis();
        Serial.printf("sending request with token %d\n", (uint32_t)lastMillis);
        Error err;
        err = MB->addRequest((uint32_t)lastMillis, _addr, READ_HOLD_REGISTER, _reg, count);
        if (err != SUCCESS)
        {
          ModbusError e(err);
          Serial.printf("Error creating request: %02X - %s\n", (int)e, (const char *)e);
        }
      }
      return val;
    }
    else if (command == "readCoils") // vout = mb.readCoils(1, \"0х0000\", 1) - "Адрес","Регистр","Кличество бит"
    {
      if (param.size())
      {
        count = (uint8_t)param[2].valD;
        count = count > 16 ? 16 : count;
        count = count < 1 ? 1 : count;
        _addr = param[0].valD;
        _reg = hexStringToUint16(param[1].valS);
        // node.begin(_addr, (Stream &)*_modbusUART);
        // val.valD = readFunctionModBus(0x01, _addr, _reg, count);

        // Read a slice of 12 coils, starting at 13
        uint32_t lastMillis = millis();
        Serial.printf("sending request with token %d\n", (uint32_t)lastMillis);
        Error err;
        err = MB->addRequest((uint32_t)lastMillis, _addr, READ_COIL, _reg, count);
        if (err != SUCCESS)
        {
          ModbusError e(err);
          Serial.printf("Error creating request: %02X - %s\n", (int)e, (const char *)e);
        }
      }
      return val;
    }
    else if (command == "readDiscreteInputs") // vout = mb.readDiscreteInputs(1, \"0х0000\", 1) - "Адрес","Регистр","Кличество бит"
    {
      if (param.size())
      {
        count = (uint8_t)param[2].valD;
        count = count > 16 ? 16 : count;
        count = count < 1 ? 1 : count;
        _addr = param[0].valD;
        _reg = hexStringToUint16(param[1].valS);
        // node.begin(_addr, (Stream &)*_modbusUART);
        // val.valD = readFunctionModBus(0x02, _addr, _reg, count);
        uint32_t lastMillis = millis();
        Serial.printf("sending request with token %d\n", (uint32_t)lastMillis);
        Error err;
        err = MB->addRequest((uint32_t)lastMillis, _addr, READ_DISCR_INPUT, _reg, count);
        if (err != SUCCESS)
        {
          ModbusError e(err);
          Serial.printf("Error creating request: %02X - %s\n", (int)e, (const char *)e);
        }
      }
      return val;
    }
    else if (command == "writeSingleRegister") // vout = mb.writeSingleRegister(1,"0x0003", 1) - addr, register, state
    {
      if (param.size())
      {
        // node.begin((uint8_t)param[0].valD, (Stream &)*_modbusUART);

        _addr = param[0].valD;
        _reg = hexStringToUint16(param[1].valS);
        // bool state = param[2].valD;
        uint16_t state = param[2].valD;
        // result = node.writeSingleRegister(_reg, state);
        if (_debug)
        {
          SerialPrint("I", "ModbusClientAsync", "writeSingleRegister, addr: " + String((uint8_t)_addr, HEX) + ", regStr: " + _regStr + ", reg: " + String(_reg, HEX) + ", state: " + String(state) + " = result: " + String(result, HEX));
        }

        // We will first set the register to a known state, read the register,
        // then write to it and finally read it again to verify the change

        // Set defined conditions first - write 0x1234 to the register
        // The Token value is used in handleData to avoid the output for this first preparation request!
        // uint32_t Token = 1111;
        uint32_t lastMillis = millis();
        Serial.printf("sending request with token %d\n", (uint32_t)lastMillis);
        Error err;
        err = MB->addRequest((uint32_t)lastMillis, _addr, WRITE_HOLD_REGISTER, _reg, state);
        if (err != SUCCESS)
        {
          ModbusError e(err);
          Serial.printf("Error creating request: %02X - %s\n", (int)e, (const char *)e);
        }
      }
      // Что можно вернуть в ответ на запись ???
      return {};
    }
    else if (command == "writeSingleCoil") // vout = mb.writeSingleCoil(1,"0x0003", 1) - addr, register, state
    {
      if (param.size())
      {
        _addr = param[0].valD;
        _reg = hexStringToUint16(param[1].valS);
        // node.begin(_addr, (Stream &)*_modbusUART);

        bool state = param[2].valD;
        // result = node.writeSingleCoil(_reg, state);
        if (_debug)
        {
          SerialPrint("I", "ModbusClientAsync", "writeSingleCoil, addr: " + String((uint8_t)_addr, HEX) + ", regStr: " + _regStr + ", reg: " + String(_reg, HEX) + ", state: " + String(state) + " = result: " + String(result, HEX));
        }

        // next set a single coil at 8
        uint32_t lastMillis = millis();
        Serial.printf("sending request with token %d\n", (uint32_t)lastMillis);
        Error err;
        err = MB->addRequest((uint32_t)lastMillis, _addr, WRITE_COIL, _reg, state);
        if (err != SUCCESS)
        {
          ModbusError e(err);
          Serial.printf("Error creating request: %02X - %s\n", (int)e, (const char *)e);
        }
      }
      // Что можно вернуть в ответ на запись койлов???
      return {};
    }
    else if (command == "writeMultipleCoils") // Пример: mb.writeMultipleCoils(1, \"0х0000\", 4, 3) - будут записаны в четыре бита 0011
    {
      if (param.size())
      {
        _addr = param[0].valD;
        _reg = hexStringToUint16(param[1].valS);
        count = (uint8_t)param[2].valD;
        count = count > 16 ? 16 : count;
        count = count < 1 ? 1 : count;
        // node.begin(_addr, (Stream &)*_modbusUART);

        uint16_t state = param[3].valD;
        // node.setTransmitBuffer(0, state);
        // result = node.writeMultipleRegisters(_reg, count);
        // node.clearTransmitBuffer();
        if (_debug)
        {
          SerialPrint("I", "ModbusClientAsync", "writeSingleCoil, addr: " + String((uint8_t)_addr, HEX) + ", regStr: " + _regStr + ", reg: " + String(_reg, HEX) + ", state: " + String(state) + " = result: " + String(result, HEX));
        }

        CoilData cd(12);
        // Finally set a a bunch of coils starting at 20
        cd = "011010010110";
        uint32_t lastMillis = millis();
        Serial.printf("sending request with token %d\n", (uint32_t)lastMillis);
        Error err;
        err = MB->addRequest((uint32_t)lastMillis, _addr, WRITE_MULT_COILS, _reg, cd.coils(), cd.size(), cd.data());
        if (err != SUCCESS)
        {
          ModbusError e(err);
          Serial.printf("Error creating request: %02X - %s\n", (int)e, (const char *)e);
        }
      }
      return {};
    }
    // На данный момент записывает 2(два) регистра!!!!! Подходит для записи float?? Функция 0х10 протокола.
    else if (command == "writeMultipleRegisters") // mb.writeMultipleRegisters(1, \"0х0000\",  1234.987)
    {
      if (param.size())
      {
        _addr = param[0].valD;
        _reg = hexStringToUint16(param[1].valS);
        // node.begin(_addr, (Stream &)*_modbusUART);

        float state = param[2].valD;

        // node.setTransmitBuffer(0, lowWord(state));
        // node.setTransmitBuffer(1, highWord(state));
        // result = node.writeMultipleRegisters(_reg, 2);
        // node.clearTransmitBuffer();
        Serial.printf("NOT SUPPORTED!\n");
        if (_debug)
        {
          SerialPrint("I", "ModbusClientAsync", "writeMultipleRegisters, addr: " + String((uint8_t)_addr, HEX) + ", reg: " + String(_reg, HEX) + ", state: " + String(state) + " (" + String(state, HEX) + ")");
        }
      }
      return {};
    }
    return val;
  }

  ~ModbusClientAsync()
  {
    delete _modbusUART;
    _modbusUART = nullptr;
  };
};

void *getAPI_ModbusRTUasync(String subtype, String param)
{
  if (subtype == F("mbClient"))
  {
    return new ModbusClientAsync(param);
  }
  {
    return nullptr;
  }
}
