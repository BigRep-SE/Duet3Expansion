/*
 * CpuTemperatureSensor.cpp
 *
 *  Created on: 8 Jun 2017
 *      Author: David
 */

#include <Movement/StepperDrivers/TMCSensor.h>
#include <Platform/Platform.h>

#if SUPPORT_TMC_RESULT

#include "CanMessageGenericParser.h"

// TODO: Create a base class with the 'Configure' call and the motor ID

// Exmaple: M308 S31  Y"tmc5160avg" P"11.0" A"Z Load[]"

GCodeResult ConfigureWithMotor(const CanMessageGenericParser& parser, const StringRef& reply, size_t &motorDriverID, int32_t &loadL, int32_t &loadH, float &scale)
{

	String<StringLength20> motorName;
	if (parser.GetStringParam('P', motorName.GetRef()) && motorName.strlen() == 1)
	{
		for(size_t _motorDriverID = 0; _motorDriverID < NumDrivers; ++_motorDriverID)
		{
			if(motorName[0] == ('0' + (char)_motorDriverID))
			{
				motorDriverID = _motorDriverID;

				int32_t _offsetVal;
				if (parser.GetIntParam('L', _offsetVal))
				{
					loadL = _offsetVal;
				}
				if (parser.GetIntParam('H', _offsetVal))
				{
					if(loadL == _offsetVal)
					{
						return GCodeResult::badOrMissingParameter;

					}
					loadH = _offsetVal;
				}
				float _scale;
				if (parser.GetFloatParam('C', _scale))
				{
					scale = _scale;
				}

				return GCodeResult::ok;
			}
		}
	}

	return GCodeResult::badOrMissingParameter;
}

// TMCMaxSensor
// Sensor type descriptors
TemperatureSensor::SensorTypeDescriptor TMCMaxSensor::typeDescriptor(TypeName, [](unsigned int sensorNum) noexcept -> TemperatureSensor *_ecv_from { return new TMCMaxSensor(sensorNum); } );

TMCMaxSensor::TMCMaxSensor(unsigned int sensorNum) : TemperatureSensor(sensorNum, "TMC Load sensor Max.")
{
}

void TMCMaxSensor::Poll()
{
	float value = scale * (float)( (int32_t)Platform::GetTmcDriversResultMax(motorDriverID) - loadL) / (float)(loadH - loadL) ;
	SetResult(value, TemperatureError::ok);
}

GCodeResult TMCMaxSensor::Configure(const CanMessageGenericParser& parser, const StringRef& reply)
{
	return ConfigureWithMotor(parser,reply,motorDriverID, loadL, loadH, scale);
}

// TMCMinSensor
// Sensor type descriptors
TemperatureSensor::SensorTypeDescriptor TMCMinSensor::typeDescriptor(TypeName, [](unsigned int sensorNum) noexcept -> TemperatureSensor *_ecv_from { return new TMCMinSensor(sensorNum); } );

TMCMinSensor::TMCMinSensor(unsigned int sensorNum) : TemperatureSensor(sensorNum, "TMC Load sensor Min.")
{
}

void TMCMinSensor::Poll()
{
	float value = scale * (float)( (int32_t)Platform::GetTmcDriversResultMin(motorDriverID) - loadL) / (float)(loadH - loadL) ;
	SetResult(value, TemperatureError::ok);
}

GCodeResult TMCMinSensor::Configure(const CanMessageGenericParser& parser, const StringRef& reply)
{
	return ConfigureWithMotor(parser,reply,motorDriverID, loadL, loadH, scale);
}

// TMCMAvgSensor
// Sensor type descriptors
TemperatureSensor::SensorTypeDescriptor TMCAvgSensor::typeDescriptor(TypeName, [](unsigned int sensorNum) noexcept -> TemperatureSensor *_ecv_from { return new TMCAvgSensor(sensorNum); } );

TMCAvgSensor::TMCAvgSensor(unsigned int sensorNum) : TemperatureSensor(sensorNum, "TMC Load sensor Avg.")
{
}

void TMCAvgSensor::Poll()
{
	float value = scale * (float)( (int32_t)Platform::GetTmcDriversResultAvg(motorDriverID) - loadL) / (float)(loadH - loadL) ;
	SetResult(value, TemperatureError::ok);
}

GCodeResult TMCAvgSensor::Configure(const CanMessageGenericParser& parser, const StringRef& reply)
{
	return ConfigureWithMotor(parser,reply,motorDriverID, loadL, loadH, scale);
}

#endif

// End
