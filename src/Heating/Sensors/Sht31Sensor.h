/*
 * Sht31Sensor.h
 *
 *  Created on: 20.04.2022
 *      Author: BigRep
 */

#ifndef SRC_HEATING_SHT31SENSOR_H_
#define SRC_HEATING_SHT31SENSOR_H_

#include "TemperatureSensor.h"

#if SUPPORT_I2C_SENSORS && SUPPORT_SHT31

#include <Platform/Platform.h>
#include <Hardware/SharedI2CClient.h>

class Sht31Sensor : public SharedI2CClient
{
public:
    static Sht31Sensor& GetInstance()
    {
        static Sht31Sensor instance;
        return instance;
    }
    Sht31Sensor(Sht31Sensor const&)     = delete;
    void operator=(Sht31Sensor const&)  = delete;
private:
    static const uint16_t I2C_ADDRESS;
    static const uint32_t I2C_TIMEOUT;
    volatile float _humidity = -273.15;
    volatile float _temperature = -273.15;
	volatile bool isInit = false;
    Sht31Sensor();  // Singleton

public:
	enum class Sht31Register : uint16_t
	{
		HighRep1MPS			= 0x2130, // Measurement with High repeatability and 1 measurement per second
		MediumRep1MPS		= 0x2126, // Measurement with Medium repeatability and 1 measurement per second
		LowRep1MPS			= 0x212D, // Measurement with Low repeatability and 1 measurement per second

		HighRep2MPS			= 0x2236, // Measurement with High repeatability and 2 measurement per second
		MediumRep2MPS		= 0x2226, // Measurement with Medium repeatability and 2 measurement per second
		LowRep2MPS			= 0x222B, // Measurement with Low repeatability and 2 measurement per second

		HighRep4MPS			= 0x2334, // Measurement with High repeatability and 4 measurement per second
		MediumRep4MPS		= 0x2322, // Measurement with Medium repeatability and 4 measurement per second
		LowRep4MPS			= 0x2329, // Measurement with Low repeatability and 4 measurement per second

		HighRep				= 0x2400, // Measurement High Repetability with Clock Stretch Disabled
		MedRep				= 0x240B, // Measurement Medium Repeatability with Clock Stretch Disabled
		LowRep				= 0x2416, // Measurement Low Repeatability with Clock Stretch Disabled

		HighRep10MPS		= 0x2737, // Measurement with High repeatability and 10 measurement per second
		MediumRep10MPS		= 0x2721, // Measurement with Medium repeatability and 10 measurement per second
		LowRep10MPS			= 0x272A, // Measurement with Low repeatability and 10 measurement per second

		HighRepStretch		= 0x2C06, // Measurement High Repeatability with Clock Stretch Enabled
		MedRepStretch		= 0x2C0D, // Measurement Medium Repeatability with Clock Stretch Enabled
		LowRepStretch		= 0x2C10, // Measurement Low Repeatability with Clock Stretch Enabled
		ClearStatus 		= 0x3041, // Clear the status
		SoftwareReset 		= 0x30A2, // Software Reset
		HeaterDisable 		= 0x3066, // Disable the Heater
		HeaterEnable 		= 0x306D, // Enable the Heater
		FetchData 			= 0xE000, // Fetch data
		ReadStatus			= 0xF32D, // Read Out of Status Register
	};

	void Init();
	TemperatureError ReadTemperature(float &temp);
	TemperatureError ReadHumidity(float &humid);
	TemperatureError DoI2CWriteRegister(const Sht31Register reg);
	TemperatureError DoI2CReadRegister(const Sht31Register reg, uint32_t &val);
	void WaitAvailable() noexcept;
private:
	[[noreturn]] static void SensorTaskCode(void* context) noexcept;
};

// This class represents a SHT31 humidity sensor
class Sht31HumiditySensor : public TemperatureSensor
{
public:
	Sht31HumiditySensor(unsigned int sensorNum);

	static constexpr const char *TypeName = "shthumidity";

	void Poll() override;

private:
	static SensorTypeDescriptor typeDescriptor;
};

// This class represents a SHT31 humidity sensor
class Sht31TemperatureSensor : public TemperatureSensor
{
public:
	Sht31TemperatureSensor(unsigned int sensorNum);

	static constexpr const char *TypeName = "shttemp";

	void Poll() override;

private:
	static SensorTypeDescriptor typeDescriptor;
};

#endif // SUPPORT_I2C_SENSORS && SUPPORT_SHT31

#endif /* SRC_HEATING_I2CTEMPERATURESENSOR_H_ */
