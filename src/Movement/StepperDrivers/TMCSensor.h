
#ifndef SRC_MOVEMENT_STEPPERDRIVERS_TMCSENSOR_H_
#define SRC_MOVEMENT_STEPPERDRIVERS_TMCSENSOR_H_

#include "Heating/Sensors/TemperatureSensor.h"

#if SUPPORT_TMC_RESULT

class TMCMaxSensor : public TemperatureSensor
{
private:
	size_t motorDriverID = NumDrivers;		// Motor ID | Use parameter 'P': Example P"11.0" (motor 0 of board 11)
	int32_t loadL = 0;					// Offset Low | Use parameter 'L'
	int32_t loadH = 0x03FF;					// Offset High | Use parameter 'H'
	float scale = 1.0f;						// Scale Factor | Use parameter 'C'
	
	static SensorTypeDescriptor typeDescriptor;

public:
	TMCMaxSensor(unsigned int sensorNum);

	static constexpr const char *TypeName = "tmcmax";

	void Poll() override;
	GCodeResult Configure(const CanMessageGenericParser& parser, const StringRef& reply) override;
};

class TMCMinSensor : public TemperatureSensor
{
private:
	size_t motorDriverID = NumDrivers;		// Motor ID | Use parameter 'P': Example P"11.0" (motor 0 of board 11)
	int32_t loadL = 0;						// Offset Low | Use parameter 'L'
	int32_t loadH = 0x03FF;					// Offset High | Use parameter 'H'
	float scale = 1.0f;						// Scale Factor | Use parameter 'C'
	
	static SensorTypeDescriptor typeDescriptor;

public:
	TMCMinSensor(unsigned int sensorNum);

	static constexpr const char *TypeName = "tmcmin";

	void Poll() override ;
	GCodeResult Configure(const CanMessageGenericParser& parser, const StringRef& reply) override;
};

class TMCAvgSensor : public TemperatureSensor
{
private:
	size_t motorDriverID = NumDrivers;		// Motor ID | Use parameter 'P': Example P"11.0" (motor 0 of board 11)
	int32_t loadL = 0;						// Offset Low | Use parameter 'L'
	int32_t loadH = 0x03FF;					// Offset High | Use parameter 'H'
	float scale = 1.0f;						// Scale Factor | Use parameter 'C'
		
	static SensorTypeDescriptor typeDescriptor;

public:
	TMCAvgSensor(unsigned int sensorNum);

	static constexpr const char *TypeName = "tmcavg";

	void Poll() override;
	GCodeResult Configure(const CanMessageGenericParser& parser, const StringRef& reply) override;
};


#endif

#endif /* SRC_MOVEMENT_STEPPERDRIVERS_TMCSENSOR_H_ */
