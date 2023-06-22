#include <Arduino.h>
#include "ADS1119.h"
#include "Streaming.h"

using namespace ADS1119ns;

// see 8.5.1.1. in data sheet. adr =64 if A0=A1=GND
ADS1119 ads1 = ADS1119(byte(0B1000000));

void setup_ads1119()
{
    Serial.println("ADS1119: Starting...");

    bool OK = false;
    ads1.begin();
    Serial.println("ADS1119: Begin");
    OK = ads1.reset();    
    Serial << "ADS1119: Reset " << (OK ? "OK" : "ERROR") << endl;
    //OK = ads1.writeConfig(config);
    setConfig(0);
    Serial << "ADS1119: Configured " << (OK ? "OK" : "ERROR") << endl;
}

void setConfig(int whichChannel)
{
    struct Configuration config;    
    //config.mux = Mux::positiveAIN0negativeAIN1;
    if (whichChannel == 0)
        config.mux = Mux::positiveAIN0negativeAGND;
    else
        config.mux = Mux::positiveAIN1negativeGND;

    // select gain
    config.gain = Gain::one;
    //config.gain = Gain::four;
    config.dataRate  = DataRate::sps20;
    //config.dataRate  = DataRate::sps90;
    config.conversionMode = ConversionMode::singleShot;    
    
    //config.voltageReference = VoltageReferenceSource::internal;
    config.voltageReference = VoltageReferenceSource::external;
    config.externalReferenceVoltage = 5.0f;

    ads1.writeConfig(config);
}

void setup()
{
    // use pin 4 as analog reference if config set to external.
    pinMode(4, OUTPUT);
    digitalWrite(4, HIGH);

    Serial.begin(9600);
    Serial.println("Starting");
    setup_ads1119();
    delay(30);
}

void loop()
{
    read_and_printVolts();
    delay(5000);
}

void read_and_printVolts()
{
    setConfig(0);
    uint16_t adc0 = ads1.readSingleADC();
    float volts0 = ads1.toVoltage(adc0);

    setConfig(1);
    uint16_t adc1 = ads1.readSingleADC();
    float volts1 = ads1.toVoltage(adc1);


    //Serial << "adc : 0x" << _HEX(adc0) << "\tV : " << _FLOAT(volts0,4) << endl;
    Serial << _FLOAT(volts0,4) << "," << _FLOAT(volts1,4) << endl;
    
    // bool ready = ads1.readStatus().dataReady == DataReady::ready;
    // Serial << "Ready :" << ready << endl;
}