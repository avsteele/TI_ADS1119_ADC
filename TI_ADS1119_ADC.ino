#include <Arduino.h>
#include "ADS1119.h"
#include "Streaming.h"

using namespace ADS1119ns;

// see 8.5.1.1. in data sheet. adr =64 if A0=A1=GND
ADS1119 ads1 = ADS1119(byte(0B1000000));

void setup_ads1119()
{
    Serial.println("ADS1119: Starting...");

    struct Configuration config;    
    //config.mux = Mux::positiveAIN0negativeAIN1;
    config.mux = Mux::positiveAIN0negativeAGND;
    
    // select gain
    config.gain = Gain::one;
    //config.gain = Gain::four;

    config.dataRate  = DataRate::sps20;
    //config.dataRate  = DataRate::sps90;

    config.conversionMode = ConversionMode::singleShot;
    
    config.voltageReference = VoltageReferenceSource::internal;
    //config.voltageReference = VoltageReferenceSource::external;
    //config.externalReferenceVoltage = 5.0f;

    bool OK = false;
    ads1.begin();
    Serial.println("ADS1119: Begin");
    OK = ads1.reset();    
    Serial << "ADS1119: Reset " << (OK ? "OK" : "ERROR") << endl;
    OK = ads1.writeConfig(config);
    Serial << "ADS1119: Configured " << (OK ? "OK" : "ERROR") << endl;
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
    uint16_t adc = ads1.readSingleADC();
    float volts = ads1.toVoltage(adc);
    Serial << "adc : 0x" << _HEX(adc) << "\tV : " << _FLOAT(volts,4) << endl;
    // bool ready = ads1.readStatus().dataReady == DataReady::ready;
    // Serial << "Ready :" << ready << endl;
}