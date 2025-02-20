/*
 * Sht31Sensor.cpp
 *
 *  Created on: 20.04.2022
 *      Author: BigRep
 */

#include <Heating/Sensors/Sht31Sensor.h>

#if SUPPORT_I2C_SENSORS && SUPPORT_SHT31

#include <Platform/Tasks.h>
#include <Platform/TaskPriorities.h>

#include <AppNotifyIndices.h>

const uint16_t Sht31Sensor::I2C_ADDRESS = 0x44;
const uint32_t Sht31Sensor::I2C_TIMEOUT = 50;

#if (SUPPORT_LIS3DH || SUPPORT_ADXL345)
#include "CommandProcessing/AccelerometerHandler.h"
#endif

constexpr size_t SHT31TaskStackWords = 200;
static Task<SHT31TaskStackWords> *sensorTask;

Sht31Sensor::Sht31Sensor() : SharedI2CClient(Platform::GetSharedI2C(),I2C_ADDRESS)
{

}

// TODO: Remove the delays

[[noreturn]] void Sht31Sensor::SensorTaskCode(void* context) noexcept
{
	Sht31Sensor* sensor = (Sht31Sensor*) context;
	while(true)
	{
		// Get temperature and humidity from register
		uint32_t valueTempAndHumidity;
		TemperatureError ret = sensor->DoI2CReadRegister(Sht31Sensor::Sht31Register::FetchData, valueTempAndHumidity);
		if(ret != TemperatureError::ok)
		{
			sensor->isInit = false;
			sensor->_humidity = -273.15;
			sensor->_temperature = -273.15;
		}
		else
		{
			// Calculate the values
			int32_t valueTempInt = (valueTempAndHumidity >> 16) & 0x0000FFFF;
			valueTempInt = ((4375 * valueTempInt) >> 14) - 4500;
			sensor->_temperature = (float)valueTempInt / 100.0f;
			int32_t valueHumidityInt = valueTempAndHumidity & 0x0000FFFF;
			valueHumidityInt = (625 * valueHumidityInt) >> 12;
			sensor->_humidity = (float)valueHumidityInt / 100.0f;
		}
		// TODO: Wake up after reading.
		delay(245);
	}
}

void Sht31Sensor::Init()
{
	if (sensorTask == nullptr)
	{
	  	sensorTask = new Task<SHT31TaskStackWords>;
		if(this->DoI2CWriteRegister(Sht31Register::SoftwareReset) == TemperatureError::ok)
		{
			if(this->DoI2CWriteRegister(Sht31Register::HighRep4MPS)  == TemperatureError::ok)
			{
				this->isInit = true;
	  			sensorTask->Create(SensorTaskCode, "DHT31", this, TaskPriority::DHTSensor);
			}
		}
	}
}

void Sht31Sensor::WaitAvailable() noexcept
{
#if (SUPPORT_LIS3DH || SUPPORT_ADXL345)
	do {
		delay(5);
	}while( AccelerometerHandler::IsRunning() );
#else
	delay(5);
#endif
}

TemperatureError Sht31Sensor::DoI2CWriteRegister(const Sht31Register reg)
{
	WaitAvailable();
	uint8_t buffer[2];
	// Split 16bit register in two uint8_t bytes, first value is the MSB
    buffer[0] = (uint8_t)((static_cast<uint16_t>(reg) >> 8) & 0xFF); 
    buffer[1] = (uint8_t)(static_cast<uint16_t>(reg) & 0xFF);    	 
	if(!Transfer(buffer,(unsigned char *)nullptr,2,0,I2C_TIMEOUT))
	{
		return TemperatureError::hardwareError;
	}
	return TemperatureError::ok;
}

TemperatureError Sht31Sensor::DoI2CReadRegister(const Sht31Register reg, uint32_t &val)
{
	if(DoI2CWriteRegister(reg) != TemperatureError::ok)
	{
		return TemperatureError::hardwareError;
	}
	unsigned char valAux[8] = {0,0,0,0,0,0,0,0};
	WaitAvailable();
	uint8_t buffer[2];
    buffer[0] = (uint8_t)((static_cast<uint16_t>(reg) >> 8) & 0xFF); 
    buffer[1] = (uint8_t)(static_cast<uint16_t>(reg) & 0xFF);    	 
	if(!Transfer(buffer,valAux,2,4,I2C_TIMEOUT))
	{
		return TemperatureError::hardwareError;
	}
	val =  ((uint32_t)valAux[0]) << 24;
	val += ((uint32_t)valAux[1]) << 16;
	val += ((uint32_t)valAux[3]) << 8;
	val += ((uint32_t)valAux[4]) << 0;
	return TemperatureError::ok;
}

TemperatureError Sht31Sensor::ReadTemperature(float &temp)
{
	if(!isInit)
	{
		return TemperatureError::hardwareError;
	}
	temp = _temperature;
	return TemperatureError::ok;
}

TemperatureError Sht31Sensor::ReadHumidity(float &humid)
{
	if(!isInit)
	{
		return TemperatureError::hardwareError;
	}
	humid = _humidity;
	return TemperatureError::ok;
}


// SHT31HUMIDITY
// Sensor type descriptors
TemperatureSensor::SensorTypeDescriptor Sht31HumiditySensor::typeDescriptor(TypeName, [](unsigned int sensorNum) noexcept -> TemperatureSensor *_ecv_from { return new Sht31HumiditySensor(sensorNum); } );

Sht31HumiditySensor::Sht31HumiditySensor(unsigned int channel) : TemperatureSensor(channel, "SHT31 Board humidity sensor")
{
}

void Sht31HumiditySensor::Poll()
{
	float humidity;
	TemperatureError ret = Sht31Sensor::GetInstance().ReadHumidity(humidity);
	if(ret != TemperatureError::ok)
	{
		SetResult(ret);
	}
	else
	{
		SetResult(humidity, TemperatureError::ok);
	}
}

// SHT31TEMPERATURE
// Sensor type descriptors
TemperatureSensor::SensorTypeDescriptor Sht31TemperatureSensor::typeDescriptor(TypeName, [](unsigned int sensorNum) noexcept -> TemperatureSensor *_ecv_from { return new Sht31TemperatureSensor(sensorNum); } );

Sht31TemperatureSensor::Sht31TemperatureSensor(unsigned int channel) : TemperatureSensor(channel, "SHT31 Board temperature sensor")
{
}

void Sht31TemperatureSensor::Poll()
{
	float temperature;
	TemperatureError ret = Sht31Sensor::GetInstance().ReadTemperature(temperature);
	if(ret != TemperatureError::ok)
	{
		SetResult(ret);
	}
	else
	{
		SetResult(temperature, TemperatureError::ok);
	}
}

#endif // SUPPORT_I2C_SENSORS && SUPPORT_SHT31

// End
