#ifndef ADS1119_H
#define ADS1119_H
/**
*  Arduino Library for Texas Instruments ADS1119 - 
*    4ch 16-Bit Analog-to-Digital Converter 
*   http://www.ti.com/lit/ds/sbas925a/sbas925a.pdf
*/

#include "Arduino.h"
#include <Wire.h>

namespace ADS1119ns
{
// default I2C address (A0=A1=GND)
#define ADS1119_DEFAULT_ADDRESS (0B1000000)

#define ADS1119_INTERNAL_REFERENCE_VOLTAGE (2.048f) 

/// @brief The adc input voltage can be scaled by gain amp
enum struct Gain: uint8_t {
	one = 0B0,
	four = 0B1
};

enum struct DataRate: uint8_t {
	sps20 = 0B00,
	sps90 = 0B01,
	sps330 = 0B10,
	sps1000 = 0B11
};

enum struct ConversionMode: uint8_t {
	singleShot = 0B0,
	continuous = 0B1
};

enum struct VoltageReferenceSource: uint8_t {
	internal = 0B0,
	external = 0B1
};

/// @brief the mux sets the positive and negative inputs to the ADC.
enum struct Mux: uint8_t {
	positiveAIN0negativeAIN1 = 0B0000,
	positiveAIN2negativeAIN3 = 0B0001, 
	positiveAIN1negativeAIN2 = 0B0010,
	positiveAIN0negativeAGND = 0B0011, 
	positiveAIN1negativeGND  = 0B0100, 
	positiveAIN2negativeAGND = 0B0101, 
	positiveAIN3negativeAGND = 0B0110,
	shortedToHalvedAVDD      = 0B0111 
};

struct Configuration
{
	Mux mux;
	Gain gain;
	// the conversion rate in samples per second
	DataRate dataRate;

	// single or continuous
	ConversionMode conversionMode; 

	// Internal 2.048-V reference selected (default), or External reference using the REFP and REFN inputs
	VoltageReferenceSource voltageReference; 

	// This is needed to convert bytes to volts. if voltageRefernce is external this should be set to the external voltage
	float externalReferenceVoltage = 0;

	/// @brief converts this configuration to a uint8_t appropriate for loading into the device register
	/// @param value 
	/// @return 
	static const Configuration fromUInt8(uint16_t value)
	{
		Configuration config;
		config.mux = Mux((value & 0B11100000) >> 5);
		config.gain = Gain((value & 0B00010000) >> 4);
		config.dataRate = DataRate((value & 0B00001100) >> 2);
		config.conversionMode = ConversionMode((value & 0B00000010) >> 1);
		config.voltageReference = VoltageReferenceSource((value & 0B00000001) >> 0);

		return config;
	}

	const uint8_t toUInt8()
	{
		uint8_t value = 0;
		value |= (uint8_t(mux) << 5);                // XXX00000
		value |= (uint8_t(gain) << 4);               // 000X0000
		value |= (uint8_t(dataRate) << 2);           // 0000XX00
		value |= (uint8_t(conversionMode) << 1);     // 000000X0
		value |= (uint8_t(voltageReference) << 0);   // 0000000X

		return value;
	}

	const float gainValue()
	{
		switch(gain)
		{
			case Gain::one : return 1.0f;
			case Gain::four : return 4.0f;
			// error
			default: return 0.0f;
		}
	}

	const float voltageReferenceValue()
	{
		switch(voltageReference)
		{
			case VoltageReferenceSource::internal : return ADS1119_INTERNAL_REFERENCE_VOLTAGE;
			case VoltageReferenceSource::external : return externalReferenceVoltage;
			// error
			default: return 0.0f;
		}
	}

	const float conversionTime_ms()
	{
		switch(dataRate)
		{
			case DataRate::sps20 : return 1000.0f / 20.0f;
			case DataRate::sps90 : return 1000.0f / 90.0f;
			case DataRate::sps330 : return 1000.0f / 330.0f;
			case DataRate::sps1000 : return 1000.0f / 1000.0f;
			// error
			default: return 0.0f;
		}
	}

	const bool operator==(const Configuration& other)
	{
		return mux==other.mux && gain==other.gain && 
		dataRate==other.dataRate && conversionMode==other.conversionMode 
		&& voltageReference==other.voltageReference;
	}
};

enum struct DataReady : uint8_t 
{
	notReady = 0B0,
	ready = 0B1
};

struct Status
{
	DataReady dataReady;

	static const Status fromUInt8(uint8_t value)
	{
		Status status;
		status.dataReady = DataReady((value & 0B10000000) >> 7);
		return status;
	}
};

/**
 * ADS1119 I2C
 * @author Adam Steele <adam@zerok.com>
 * @brief I2C communication with Texas instruments ADS1119.
 */
class ADS1119
{
public:
	ADS1119(uint8_t address = ADS1119_DEFAULT_ADDRESS);

	/// @brief Initialize the I2C bus
	/// @param theWire 
	void begin(TwoWire *theWire = &Wire);

	/// @brief triggers a single ADC conversion, waits for it to complete, 
	///  and returns the converted value
	/// @return the 16bit ADC value
	uint16_t readSingleADC();

	/// @brief triggers a single ADC conversion, waits for it to complete, and 
	/// returns the converted value in volts based on the last set configuration
	/// @return 
	float readSingleVoltage();

	/// @brief converts a 16b ADC value to a voltage (based on the reference voltage in the configuration)
	float const toVoltage(uint16_t adcValue);

	/// @brief  places the device into power-down mode. Register values are retained. START/SYNC to wake
	/// @details See data sheet 8.5.3.4
	/// @return true if no errors, false otherwise
	bool powerDown();

	/// @brief resets the device to the default state
	/// @details See data sheet sec 8.5.3.2
	/// @return true if no errors, false otherwise
	bool reset();

	/// @brief if in single acquisition mode, this command will start a conversion
	/// @return true if no errors, false otherwise
	bool startSync();

	/// @brief writes the configuration to the device
	/// @param config 
	/// @return true if no errors, false otherwise
	bool writeConfig(Configuration config);

	/// @brief reads the configuration currently set on the device
	/// @return 
	Configuration readConfig();

	/// @brief read the last ADC value converted
	/// @return the 16b ADC value
	uint16_t readLastAdc();

	Status readStatus();
private:
	TwoWire *_i2c;
	uint8_t _address;
	Configuration _config;
	
	/// @brief writes to a single 8-bit register
	/// @param registerID 
	/// @param value 
	/// @return 
	bool writeRegister(uint8_t registerID, uint8_t value);
	/// @brief reads a single 8-bit register
	/// @param registerToRead 
	/// @return 
	uint8_t readRegister(uint16_t registerToRead);

	/// @brief writes a single byte to the device and ends transmission
	/// @param value 
	/// @return 
	bool writeByte(uint8_t value);
};

}
#endif