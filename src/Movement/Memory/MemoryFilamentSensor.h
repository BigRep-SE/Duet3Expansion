/*
 * MetersSensor.h
 *
 *  Created on: 31 Oct 2023
 *      Author: m.fabbro
 */

#ifndef SRC_MOVEMENT_MEMORY_MEMORYFILAMENTSENSOR_H_
#define SRC_MOVEMENT_MEMORY_MEMORYFILAMENTSENSOR_H_

#include <Heating/Sensors/TemperatureSensor.h>

#if SUPPORT_EEPROM_MEMORY

class MemoryFilamentForwardSensor : public TemperatureSensor
{
public:
	MemoryFilamentForwardSensor(unsigned int sensorNum);

	static constexpr const char *TypeName = "odomforw";

	void Poll() override;

private:
	static SensorTypeDescriptor typeDescriptor;
	
};

class MemoryFilamentBackwardSensor : public TemperatureSensor
{
public:
	MemoryFilamentBackwardSensor(unsigned int sensorNum);

	static constexpr const char *TypeName = "odomback";

	void Poll() override;

private:
	static SensorTypeDescriptor typeDescriptor;

};



#endif


#endif /* SRC_MOVEMENT_MEMORY_MEMORYFILAMENTSENSOR_H_ */
