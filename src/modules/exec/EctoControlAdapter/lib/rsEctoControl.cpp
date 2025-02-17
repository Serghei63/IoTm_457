#include <Arduino.h>
#include <ArduinoJson.h>
#ifdef ESP32
#include <rom/spi_flash.h>
#endif
#include "rsEctoControl.h"

struct IoTValue
{
    float valD = 0;
    String valS = "";

    bool isDecimal = true;
};
uint8_t _DIR_PIN_ = 0;

void modbusPreTransmission()
{
    //  delay(500);
    if (_DIR_PIN_)
        digitalWrite(_DIR_PIN_, HIGH);
}

// Pin 4 made low for Modbus receive mode
// Контакт 4 установлен на низком уровне для режима приема Modbus
void modbusPostTransmission()
{
    if (_DIR_PIN_)
        digitalWrite(_DIR_PIN_, LOW);
    //  delay(500);
}

//=======================================================================================================
// setup()
RsEctoControl::RsEctoControl(String parameters)
{
    node.preTransmission(modbusPreTransmission);
    node.postTransmission(modbusPostTransmission);

    if (parameters != "")
    {
        DynamicJsonDocument doc(4096);
        DeserializationError error = deserializeJson(doc, parameters);
        if (!error)
        {
            if (doc.containsKey("DIR_PIN"))
                _DIR_PIN = doc["DIR_PIN"].as<int>();
            if (doc.containsKey("addr"))
                _addr = doc["addr"].as<int>();
            if (doc.containsKey("debug"))
                _debug = doc["debug"].as<int>();
        }
    }
    if (_DIR_PIN)
    {
        _DIR_PIN_ = _DIR_PIN;
        pinMode(_DIR_PIN, OUTPUT);
        digitalWrite(_DIR_PIN, LOW);
    }

    // 0x14 – адаптер OpenTherm (вторая версия)
    // 0x15 – адаптер eBus
    // 0x16 – адаптер Navien
    uint16_t type;
    readFunctionModBus(0x0000, type);
    if (0x14 != (uint8_t)type || 0x15 != (uint8_t)type || 0x16 != (uint8_t)type)
    {
        _SerialPrint("E", "EctoControlAdapter", "Не подходящее устройство, type: " + String(type, HEX));
    }

    uint8_t addr = node.readAddresEctoControl();
    _SerialPrint("I", "EctoControlAdapter", "readAddresEctoControl, addr: " + String(addr, HEX) + " - Enter to configuration");

    getModelVersion();
    getBoilerInfo();
    getBoilerStatus();
}

void RsEctoControl::begin(uint8_t slave, Stream &serial)
{
    _modbusUART = &serial;
    node.begin(slave, serial);
}

void RsEctoControl::initFunction(void (*publishData)(String, String), void (*sendTelegramm)(String), void (*SerialPrint)(const String &, const String &, const String &))
{ //, const String &
    _publishData = publishData;
    _sendTelegramm = sendTelegramm;
    _SerialPrint = SerialPrint;
};

void RsEctoControl::doByInterval()
{
    // readBoilerInfo();
    getBoilerStatus();

    getCodeError();
    getCodeErrorExt();
    if (info.adapterType == 0)
        getFlagErrorOT();
    // getFlowRate();
    // getMaxSetCH();
    // getMaxSetDHW();
    // getMinSetCH();
    // getMinSetDHW();
    getModLevel();
    getPressure();
    getTempCH();
    getTempDHW();
    getTempOutside();
}

// Основной цикл программы
void RsEctoControl::loop(bool isNetworkActive, bool mqttIsConnect)
{
    _isNetworkActive = isNetworkActive;
    _mqttIsConnect = mqttIsConnect;
}

// Исполнительные комманды
void RsEctoControl::execute(String command, std::vector<IoTValue> &param)
{
}

bool RsEctoControl::writeFunctionModBus(const uint16_t &reg, uint16_t &data)
{
    // set word 0 of TX buffer to least-significant word of counter (bits 15..0)
    node.setTransmitBuffer(1, lowWord(data));
    // set word 1 of TX buffer to most-significant word of counter (bits 31..16)
    node.setTransmitBuffer(0, highWord(data));
    // slave: write TX buffer to (2) 16-bit registers starting at register 0
    uint8_t result = node.writeMultipleRegisters(0, 2);
    if (_debug > 2)
    {
        _SerialPrint("I", "EctoControlAdapter", "writeSingleRegister, addr: " + String((uint8_t)_addr, HEX) + ", reg: " + String(reg, HEX) + ", state: " + String(data) + " = result: " + String(result, HEX));
    }
}

bool RsEctoControl::readFunctionModBus(const uint16_t &reg, uint16_t &reading)
{
    // float retValue = 0;
    if (_modbusUART)
    {
        // if (!addr)
        //    addr = _addr;
        node.begin(_addr, (Stream &)*_modbusUART);
        uint8_t result;
        // uint32_t reading;

        if (reg == 0x0000)
            result = node.readHoldingRegisters(reg, 4);
        else
            result = node.readHoldingRegisters(reg, 1);
        if (_debug > 2)
            _SerialPrint("I", "EctoControlAdapter", "readHoldingRegisters, addr: " + String(_addr, HEX) + ", reg: " + String(reg, HEX) + " = result: " + String(result, HEX));
        // break;
        if (result == node.ku8MBSuccess)
        {
            if (reg == 0x0000)
            {
                reading = node.getResponseBuffer(0x03);
                reading = (uint16_t)((uint8_t)(reading) >> 8);
                _SerialPrint("I", "EctoControlAdapter", "read info, addr: " + String(_addr, HEX) + ", type: " + String(reading, HEX));
            }
            else
            {
                reading = node.getResponseBuffer(0x00);
                if (_debug > 2)
                    _SerialPrint("I", "EctoControlAdapter", "Success, Received data, register: " + String(reg) + " = " + String(reading, HEX));
            }
            node.clearResponseBuffer();
        }
        else
        {
            if (_debug > 2)
                _SerialPrint("E", "EctoControlAdapter", "Failed, Response Code: " + String(result, HEX));
            return false;
        }

        if (reading != 0x7FFF)
            return true;
        else
            return false;
    }
    return false;
}

bool RsEctoControl::getModelVersion()
{
    uint16_t reqData;
    info.boilerMemberCode = readFunctionModBus(ReadDataEctoControl::ecR_MemberCode, info.boilerMemberCode);
    info.boilerModelCode = readFunctionModBus(ReadDataEctoControl::ecR_ModelCode, info.boilerModelCode);
    bool ret = readFunctionModBus(ReadDataEctoControl::ecR_AdaperVersion, reqData);
    info.adapterHardVer = highByte(reqData);
    info.adapterSoftVer = lowByte(reqData);
    return ret;
}

bool RsEctoControl::getBoilerInfo()
{
    uint16_t reqData;
    bool ret = readFunctionModBus(ReadDataEctoControl::ecR_AdapterInfo, reqData);
    info.adapterType = highByte(reqData) & 0xF8;
    info.boilerStatus = (highByte(reqData) >> 3) & 1u;
    info.rebootStatus = lowByte(reqData);
    if (ret)
    {
        _publishData("adapterType", String(info.adapterType));
        _publishData("boilerStatus", String(info.boilerStatus));
        _publishData("rebootStatus", String(info.rebootStatus));
    }
    return ret;
}
bool RsEctoControl::getBoilerStatus()
{
    uint16_t reqData;
    bool ret = readFunctionModBus(ReadDataEctoControl::ecR_BoilerStatus, reqData);
    status.burnStatus = (lowByte(reqData) >> 0) & 1u;
    status.CHStatus = (lowByte(reqData) >> 1) & 1u;
    status.DHWStatus = (lowByte(reqData) >> 2) & 1u;
    if (ret)
    {
        _publishData("burnStatus", String(status.burnStatus));
        _publishData("CHStatus", String(status.CHStatus));
        _publishData("DHWStatus", String(status.DHWStatus));
    }
    return ret;
}
bool RsEctoControl::getCodeError()
{
    bool ret = readFunctionModBus(ReadDataEctoControl::ecR_CodeError, code);
    if (ret)
    {
        _publishData("codeError", String(code));
        if (codeExt)
            _sendTelegramm("EctoControlAdapter: код ошибки: " + String((int)codeExt));
    }
    return ret;
}
bool RsEctoControl::getCodeErrorExt()
{
    bool ret = readFunctionModBus(ReadDataEctoControl::ecR_CodeErrorExt, codeExt);
    if (ret)
    {
        _publishData("codeErrorExt", String(codeExt));
        if (codeExt)
            _sendTelegramm("EctoControlAdapter: код ошибки: " + String((int)codeExt));
    }
    return ret;
}
bool RsEctoControl::getFlagErrorOT()
{
    uint16_t reqData;
    bool ret = readFunctionModBus(ReadDataEctoControl::ecR_FlagErrorOT, reqData);
    flagErr = lowByte(reqData);
    if (ret)
    {
        _publishData("flagErr", String(flagErr));
        switch (flagErr)
        {
        case 0:
            _sendTelegramm("EctoControlAdapter: Необходимо обслуживание!");
            break;
        case 1:
            _sendTelegramm("EctoControlAdapter: Котёл заблокирован!");
            break;
        case 2:
            _sendTelegramm("EctoControlAdapter: Низкое давление в отопительном контуре!");
            break;
        case 3:
            _sendTelegramm("EctoControlAdapter: Ошибка розжига!");
            break;
        case 4:
            _sendTelegramm("EctoControlAdapter: Низкое давления воздуха!");
            break;
        case 5:
            _sendTelegramm("EctoControlAdapter: Перегрев теплоносителя в контуре!");
            break;
        default:
            break;
        }
    }
    return ret;
}

bool RsEctoControl::getFlowRate()
{
    uint16_t reqData;
    bool ret = readFunctionModBus(ReadDataEctoControl::ecR_FlowRate, reqData);
    flow = lowByte(reqData) / 10.f;
    if (ret)
        _publishData("flowRate", String(flow));
    return ret;
}
bool RsEctoControl::getMaxSetCH()
{
    uint16_t reqData;
    bool ret = readFunctionModBus(ReadDataEctoControl::ecR_MaxSetCH, reqData);
    maxSetCH = (float)lowByte(reqData);
    if (ret)
        _publishData("maxSetCH", String(maxSetCH));
    return ret;
}
bool RsEctoControl::getMaxSetDHW()
{
    uint16_t reqData;
    bool ret = readFunctionModBus(ReadDataEctoControl::ecR_MaxSetDHW, reqData);
    maxSetDHW = (float)lowByte(reqData);
    if (ret)
        _publishData("maxSetDHW", String(maxSetDHW));
    return ret;
}
bool RsEctoControl::getMinSetCH()
{
    uint16_t reqData;
    bool ret = readFunctionModBus(ReadDataEctoControl::ecR_MinSetCH, reqData);
    minSetCH = (float)lowByte(reqData);
    if (ret)
        _publishData("minSetCH", String(minSetCH));
    return ret;
}
bool RsEctoControl::getMinSetDHW()
{
    uint16_t reqData;
    bool ret = readFunctionModBus(ReadDataEctoControl::ecR_MinSetDHW, reqData);
    minSetDHW = (float)lowByte(reqData);
    if (ret)
        _publishData("minSetDHW", String(minSetDHW));
    return ret;
}
bool RsEctoControl::getModLevel()
{
    uint16_t reqData;
    bool ret = readFunctionModBus(ReadDataEctoControl::ecR_ModLevel, reqData);
    modLevel = (float)lowByte(reqData);
    if (ret)
        _publishData("modLevel", String(modLevel));
    return ret;
}
bool RsEctoControl::getPressure()
{
    uint16_t reqData;
    bool ret = readFunctionModBus(ReadDataEctoControl::ecR_Pressure, reqData);
    press = lowByte(reqData) / 10.f;
    if (ret)
        _publishData("press", String(press));
    return ret;
}
bool RsEctoControl::getTempCH()
{
    uint16_t reqData;
    bool ret = readFunctionModBus(ReadDataEctoControl::ecR_TempCH, reqData);
    tCH = reqData / 10.f;
    if (ret)
        _publishData("tCH", String(tCH));
    return ret;
}
bool RsEctoControl::getTempDHW()
{
    uint16_t reqData;
    bool ret = readFunctionModBus(ReadDataEctoControl::ecR_TempDHW, reqData);
    tDHW = reqData / 10.f;
    if (ret)
        _publishData("tDHW", String(tDHW));
    return ret;
}
bool RsEctoControl::getTempOutside()
{
    uint16_t reqData;
    bool ret = readFunctionModBus(ReadDataEctoControl::ecR_TempOutside, reqData);
    tOut = (float)lowByte(reqData);
    if (ret)
        _publishData("tOut", String(tOut));
    return ret;
}

bool RsEctoControl::setTypeConnect(float &data)
{
    bool ret = false;
    if (writeFunctionModBus(ecW_SetTypeConnect, (uint16_t &)data))
    {
        // TODO запросить результат записи у адаптера
        ret = true;
    }
    else
    {
        if (_debug > 1)
            _SerialPrint("E", "EctoControlAdapter", "Failed, setTypeConnect");
    }
    return ret;
}
bool RsEctoControl::setTCH(float &data)
{
    bool ret = false;
    uint16_t d16 = data * 10;
    if (writeFunctionModBus(ecW_TSetCH, d16))
    {
        ret = true;
    }
    else
    {
        if (_debug > 1)
            _SerialPrint("E", "EctoControlAdapter", "Failed, setTCH");
    }

    return ret;
}
bool RsEctoControl::setTCHFaultConn(float &data)
{
    bool ret = false;
    uint16_t d16 = data * 10;
    if (writeFunctionModBus(ecW_TSetCHFaultConn, d16))
    {
        ret = true;
    }
    else
    {
        if (_debug > 1)
            _SerialPrint("E", "EctoControlAdapter", "Failed, setTCHFaultConn");
    }
    return ret;
}
bool RsEctoControl::setMinCH(float &data)
{
    bool ret = false;
    if (writeFunctionModBus(ecW_TSetMinCH, (uint16_t &)data))
    {
        ret = true;
    }
    else
    {
        if (_debug > 1)
            _SerialPrint("E", "EctoControlAdapter", "Failed, setMinCH");
    }
    return ret;
}
bool RsEctoControl::setMaxCH(float &data)
{
    bool ret = false;
    if (writeFunctionModBus(ecW_TSetMaxCH, (uint16_t &)data))
    {
        ret = true;
    }
    else
    {
        if (_debug > 1)
            _SerialPrint("E", "EctoControlAdapter", "Failed, setMaxCH");
    }
    return ret;
}
bool RsEctoControl::setMinDHW(float &data)
{
    bool ret = false;
    if (writeFunctionModBus(ecW_TSetMinDHW, (uint16_t &)data))
    {
        ret = true;
    }
    else
    {
        if (_debug > 1)
            _SerialPrint("E", "EctoControlAdapter", "Failed, setMinDHW");
    }
    return ret;
}
bool RsEctoControl::setMaxDHW(float &data)
{
    bool ret = false;
    if (writeFunctionModBus(ecW_TSetMaxDHW, (uint16_t &)data))
    {
        ret = true;
    }
    else
    {
        if (_debug > 1)
            _SerialPrint("E", "EctoControlAdapter", "Failed, setMaxDHW");
    }
}
bool RsEctoControl::setTDHW(float &data)
{
    bool ret = false;
    if (writeFunctionModBus(ecW_TSetDHW, (uint16_t &)data))
    {
        ret = true;
    }
    else
    {
        if (_debug > 1)
            _SerialPrint("E", "EctoControlAdapter", "Failed, setTDHW");
    }
    return ret;
}
bool RsEctoControl::setMaxModLevel(float &data)
{
    bool ret = false;
    if (writeFunctionModBus(ecW_SetMaxModLevel, (uint16_t &)data))
    {
        ret = true;
    }
    else
    {
        if (_debug > 1)
            _SerialPrint("E", "EctoControlAdapter", "Failed, setMaxModLevel");
    }
    return ret;
}

bool RsEctoControl::setStatusCH(bool data)
{
    bool ret = false;
    enableCH = data;
    uint16_t stat = enableCH | (enableDHW << 1) | (enableCH2 << 2);
    if (writeFunctionModBus(ecW_SetStatusBoiler, stat))
    {
        ret = true;
    }
    else
    {
        if (_debug > 1)
            _SerialPrint("E", "EctoControlAdapter", "Failed, setStatusCH");
    }
    return ret;
}
bool RsEctoControl::setStatusDHW(bool data)
{
    bool ret = false;
    enableDHW = data;
    uint16_t stat = enableCH | (enableDHW << 1) | (enableCH2 << 2);
    if (writeFunctionModBus(ecW_SetStatusBoiler, stat))
    {
        ret = true;
    }
    else
    {
        if (_debug > 1)
            _SerialPrint("E", "EctoControlAdapter", "Failed, setStatusDHW");
    }
    return ret;
}
bool RsEctoControl::setStatusCH2(bool data)
{
    bool ret = false;
    enableCH2 = data;
    uint16_t stat = enableCH | (enableDHW << 1) | (enableCH2 << 2);
    if (writeFunctionModBus(ecW_SetStatusBoiler, stat))
    {
        ret = true;
    }
    else
    {
        if (_debug > 1)
            _SerialPrint("E", "EctoControlAdapter", "Failed, setStatusCH2");
    }
    return ret;
}

bool RsEctoControl::lockOutReset()
{
    bool ret = false;
    uint16_t d16 = comm_LockOutReset;
    if (writeFunctionModBus(ecW_Command, d16))
    {
        ret = true;
    }
    else
    {
        if (_debug > 1)
            _SerialPrint("E", "EctoControlAdapter", "Failed, lockOutReset");
    }
    return ret;
}
bool RsEctoControl::rebootAdapter()
{
    bool ret = false;
    uint16_t d16 = comm_RebootAdapter;
    if (writeFunctionModBus(ecW_Command, d16))
    {
        ret = true;
    }
    else
    {
        if (_debug > 1)
            _SerialPrint("E", "EctoControlAdapter", "Failed, rebootAdapter");
    }
    return ret;
}

const String RsEctoControl::getChipId()
{
    return String(ESP_getChipId()) + "-" + String(getFlashChipIdNew()); // + "v" + String(FIRMWARE_VERSION);
}

uint32_t RsEctoControl::getFlashChipIdNew()
{
    // Cache since size does not change
    static uint32_t flashChipId = 0;

    if (flashChipId == 0)
    {
#ifdef ESP32
        uint32_t tmp = g_rom_flashchip.device_id;

        for (int i = 0; i < 3; ++i)
        {
            flashChipId = flashChipId << 8;
            flashChipId |= (tmp & 0xFF);
            tmp = tmp >> 8;
        }

        //    esp_flash_read_id(nullptr, &flashChipId);
#elif defined(ESP8266)
        flashChipId = ESP.getFlashChipId();
#endif // ifdef ESP32
    }
    return flashChipId;
}

uint32_t RsEctoControl::ESP_getChipId(void)
{
#ifdef ESP32
    uint32_t id = 0;
    for (uint32_t i = 0; i < 17; i = i + 8)
    {
        id |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }
    return id;
#else
    return ESP.getChipId();
#endif
}

RsEctoControl::~RsEctoControl()
{
}
