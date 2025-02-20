/*
 * AdcStreamHandler.h
 *
 *  Created on: 14 Jul 2023
 *      Author: BigRep
 */

#ifndef SRC_COMMANDPROCESSING_ADCSTREAMHANDLER_H_
#define SRC_COMMANDPROCESSING_ADCSTREAMHANDLER_H_

#include <RepRapFirmware.h>

#if SUPPORT_ADCSTREAM
#include <Platform/Platform.h>

class CanMessageGeneric;
class CanMessageStartAccelerometer;

typedef AveragingFilter<ThermistorReadingsAveraged> adcFastStream;

// Interface to the accelerometer. These functions are called by the main task.
namespace AdcStreamHandler
{
	void Init(int stream) noexcept;
	bool IsPresent() noexcept;
	bool IsRunning() noexcept; // We need this to disable anything else in the I2C
	GCodeResult ProcessConfigRequest(const CanMessageGeneric& msg, const StringRef& reply) noexcept;
	GCodeResult ProcessStartRequest(const CanMessageStartAccelerometer& msg, const StringRef& reply) noexcept;
	void Diagnostics(const StringRef& reply) noexcept;
};

#endif

#endif /* SRC_COMMANDPROCESSING_ADCSTREAMHANDLER_H_ */
