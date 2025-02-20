/*
 * LedHandler.h
 *
 *  Created on: 01.08.2022
 *      Author: BigRep
 */

#ifndef SRC_LEDSTRIPS_STATES_LEDHANDLER_H_
#define SRC_LEDSTRIPS_STATES_LEDHANDLER_H_

#include <RepRapFirmware.h>

#if SUPPORT_LED_STATES
# include <CAN/CanInterface.h>

class CanMessageGenericParser;

namespace LedHandler
{
	enum class SignalingState : uint8_t {
		Off 			= 0 ,		///< Turn off all the LEDs
		Error 			= 1 ,		///< To report an error in the extruder in the machine
		Idle 			= 2 ,		///< To use when the extruder is in idle (but connected)
		Selected 		= 3 ,		///< To use when the extruder is selected
		Warning			= 4 ,		///< Warning state
		OutOfFilament 	= 5 ,		///< To report an out-of-filament problem in the extruder
		Heating 		= 6 ,		///< (not use)The temperature is going up
		Cooling 		= 7 ,		///< (not use) The temperature is going down
		OnTemperature 	= 8 ,		///< (not use) The target temperature was reached
		Manual			= 9 , 		///< User defined color using RGB
		MAX
	};

	void Init() noexcept;
	void SetState(SignalingState state) noexcept;
	GCodeResult HandleM950(CanMessageGenericParser& parser, const StringRef& reply) noexcept;
	GCodeResult HandleM150(CanMessageGenericParser& parser, const StringRef& reply) noexcept;
	/*
	void SetState(uint8_t state);
	*/
};

#endif

#endif /* SRC_LEDSTRIPS_STATES_LEDHANDLER_H_ */
