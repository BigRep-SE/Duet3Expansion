#include <Movement/Memory/MemoryFilamentSensor.h>

#if SUPPORT_EEPROM_MEMORY

#include <Movement/Memory/MovementMemoryHandler.h>

// MemoryFilamentForwardSensor
// Sensor type descriptors
TemperatureSensor::SensorTypeDescriptor MemoryFilamentForwardSensor::typeDescriptor(TypeName, [](unsigned int sensorNum) noexcept -> TemperatureSensor *_ecv_from { return new MemoryFilamentForwardSensor(sensorNum); } );

MemoryFilamentForwardSensor::MemoryFilamentForwardSensor(unsigned int channel) : TemperatureSensor(
		channel, "Meters Forward Sensor") {
}

void MemoryFilamentForwardSensor::Poll()
{
	if(!MovementMemoryHandler::IsInit()){
		SetResult(TemperatureError::notInitialised);
	}
	else{
		SetResult(((float)MovementMemoryHandler::GetForwardMeters()) / 1000.0f, TemperatureError::ok);
	}
}

// MemoryFilamentBackwardSensor
// Sensor type descriptors
TemperatureSensor::SensorTypeDescriptor MemoryFilamentBackwardSensor::typeDescriptor(TypeName, [](unsigned int sensorNum) noexcept -> TemperatureSensor *_ecv_from { return new MemoryFilamentBackwardSensor(sensorNum); } );

MemoryFilamentBackwardSensor::MemoryFilamentBackwardSensor(unsigned int channel) : TemperatureSensor(
		channel, "Meters Backward Sensor") {
}

void MemoryFilamentBackwardSensor::Poll()
{
	if( !MovementMemoryHandler::IsInit() ) {
		SetResult(TemperatureError::notInitialised);
	}
	else{
		SetResult(((float)MovementMemoryHandler::GetBackwardMeters()) / 1000.0f, TemperatureError::ok);
	}
}


#endif
