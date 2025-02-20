#ifndef __SRC_HEATING_SENSORS__
#define __SRC_HEATING_SENSORS__
#include "Heating/Sensors/TemperatureSensor.h"

class FilteredSensor : public TemperatureSensor { // IIR First order Low-Pass filter

public:
	FilteredSensor(unsigned int sensorNum,const char *t) noexcept : TemperatureSensor(sensorNum, t) {};

	GCodeResult ConfigureFilter(const CanMessageGenericParser& parser, bool &changed) ;

protected:
	static constexpr float MinK = 0.01f;
	static constexpr float MaxK = 0.99f;

	float FilterValue( float input )
	{
		value = ( (kIIR >= MaxK) ?
				input : // Filter is disabled
				value * (1.0f - kIIR) + input * kIIR );
		return value ;
	};
	float GetK() const noexcept { return kIIR; };
private:
	void SetK(float k) noexcept;

	float kIIR = 1.0f; 	// k value of First Order IIR filter. Default is IIR filter is disabled.
	float value = 0.0f;
};

#endif
