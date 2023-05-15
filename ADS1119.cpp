#include "ADS1119.h"
#include "Arduino.h"
#include <Wire.h>

namespace ADS1119ns
{

// Refer to COMMANDS Section 8.5.3 of
// http://www.ti.com/lit/ds/sbas925a/sbas925a.pdf
#define ADS1119_RESET 			(0B00000110)
#define ADS1119_STARTSYNC 		(0B00001000)
#define ADS1119_POWERDOWN 		(0B00000010)
#define ADS1119_RDATA 			(0B00010000)
#define ADS1119_READ_CONFIG_REGISTER	(0B00100000)
#define ADS1119_READ_STATUS_REGISTER	(0B00100100)
#define ADS1119_WRITE_CONFIG_REGISTER	(0B01000000)

ADS1119::ADS1119(uint8_t address) 
{
    _address = address;
}

void ADS1119::begin(TwoWire *theWire) 
{
    _i2c = theWire;
    _i2c->begin();
}

bool ADS1119::reset() 
{
    return writeByte(ADS1119_RESET);
}

bool ADS1119::powerDown() 
{
    return writeByte(ADS1119_POWERDOWN);
}

bool ADS1119::startSync()
{
    return writeByte(ADS1119_STARTSYNC);
}

bool ADS1119::writeConfig(Configuration config)
{
    uint16_t value = config.toUInt8();
    writeRegister(ADS1119_WRITE_CONFIG_REGISTER, value);
    // confirm the written value is the same as the device returns now when queried
    Configuration configOut = readConfig();    
    bool OK = config == configOut;
    // update stored config (used for conversion to voltage)
    //    cant use configOut since its reference voltage is not set
    _config = config;
    return OK;
}

Configuration ADS1119::readConfig()
{
    // last bit is 1 to indicate a READ (see sec 8.5.3.6)
    uint8_t value = readRegister(ADS1119_READ_CONFIG_REGISTER);
    return Configuration::fromUInt8(value);
}

float const ADS1119::toVoltage(uint16_t adc)
{
    float gain = _config.gainValue();
    float referenceVoltage = _config.voltageReferenceValue();  
    float voltage = referenceVoltage * (float((int16_t)adc) / (0x7FFF)) / gain;

    return voltage;
}

uint16_t ADS1119::readSingleADC() 
{
    startSync();
    delay(_config.conversionTime_ms());
    return readLastAdc();
}

float ADS1119::readSingleVoltage() 
{
    uint16_t adc = readSingleADC();
    return toVoltage(adc);
}

uint16_t ADS1119::readLastAdc()
{
    writeByte(ADS1119_RDATA);
    _i2c->requestFrom(_address, (uint8_t)2);

    if (_i2c->available() < 2) {
         reset();
         return 0x0;
    }
    int byte1 = _i2c->read();    
    int byte2 = _i2c->read();

    return ((byte1 << 8) | byte2);
}

bool ADS1119::writeRegister(uint8_t registerValue, uint8_t value)
{
    _i2c->beginTransmission(_address);
    _i2c->write(registerValue);
    _i2c->write(value);

    return _i2c->endTransmission() == 0;
}

bool ADS1119::writeByte(uint8_t value)
{
    _i2c->beginTransmission(_address);
    _i2c->write(value);

    return _i2c->endTransmission() == 0;
}

uint8_t ADS1119::readRegister(uint16_t registerToRead)
{
    writeByte(registerToRead);
    delay(1);
    _i2c->requestFrom(_address, (uint8_t)1);
    return _i2c->read();
}

Status ADS1119::readStatus()
{
    uint8_t value = readRegister(ADS1119_READ_STATUS_REGISTER);
    return Status::fromUInt8(value);
}
} // namespace ADS1119ns