/*
 * LedHandler.cpp
 *
 *  Created on: 04.08.2022
 *      Author: BigRep
 */

#include "LedHandler.h"

#if SUPPORT_LED_STATES
#include "NeoPixelStrip.h"

#include <RTOSIface/RTOSIface.h>
#include <CanMessageFormats.h>
#include <Platform/Platform.h>
#include <Platform/TaskPriorities.h>
#include <CanMessageBuffer.h>
#include <CAN/CanInterface.h>
#include <CanMessageGenericParser.h>
#include <CanMessageGenericTables.h>

#include <hri_sercom_e54.h>
#include "DmacManager.h"
#include "Serial.h"

#include "Movement/StepTimer.h"

constexpr size_t LedTaskStackWords = 200;
static Task<LedTaskStackWords> *ledTask;

constexpr uint32_t LedTaskLoopDelayMs = 40;
constexpr uint32_t LedTaskStartUpDelayMs = 500;

using namespace NeoPixelColors;

uint16_t stripId = MaxLedStrips;
volatile LedHandler::SignalingState currentState {LedHandler::SignalingState::Off};

volatile uint8_t userRed{0}, userGreen{0}, userBlue{0} ;
volatile uint8_t userEffect{0}; // TODO: this should be an enum
volatile uint32_t userEffectTime{0};

namespace SignalingStateColor{
	const NeoPixelStrip::Color Off 				= NeoPixelColors::Black;
	const NeoPixelStrip::Color Error 			= NeoPixelColors::Red;
	const NeoPixelStrip::Color NotInitialized	= NeoPixelColors::Green;
	const NeoPixelStrip::Color Disconnected		= NeoPixelColors::Cyan;
	const NeoPixelStrip::Color Idle		 		= NeoPixelColors::White;
	const NeoPixelStrip::Color Selected	 		= NeoPixelColors::Green;
	const NeoPixelStrip::Color Warning	 		= NeoPixelColors::Orange;
	const NeoPixelStrip::Color OutOfFilament 	= NeoPixelColors::Purple;
	const NeoPixelStrip::Color Heating 			= NeoPixelColors::Red;
	const NeoPixelStrip::Color Cooling 			= NeoPixelColors::Blue;
	const NeoPixelStrip::Color OnTemperature 	= NeoPixelColors::Red;
};

class StripColorHandler {
public:
	/*
	 * A single Led can be set.
	 */
	void Set(const uint8_t led, const NeoPixelStrip::Color &color){
		if(led < NeoPixelStrip::AmountOfLEDs){
			toSet[led] = color;
		}
	};

	/*
	 * All the LEDs will be set to the same color.
	 */
	void SetAll(const NeoPixelStrip::Color &color){
		for(size_t i = 0; i < NeoPixelStrip::AmountOfLEDs; ++i) {
			toSet[i] = color;
		}
	};

	/*
	 * We will just update the strip if there is a change in any of the
	 * leds.
	 */
	void Update(){
		bool needsUpdate = false;
		for(size_t i = 0; i < NeoPixelStrip::AmountOfLEDs; ++i) {
			if( prev[i] != toSet[i] ) {
				needsUpdate = true;
				prev[i] = toSet[i];
				ledStrip.SetColor(i, prev[i]);
			}
		}
		if(needsUpdate) {
			ledStrip.UpdateStrip();
		}
	};

	StripColorHandler() {
		Init();
	};

private:
	NeoPixelStrip ledStrip;

	NeoPixelStrip::Color toSet[NeoPixelStrip::AmountOfLEDs];
	NeoPixelStrip::Color prev[NeoPixelStrip::AmountOfLEDs];

	void Init() {
		ledStrip.Init();

		for(size_t i = 0; i < NeoPixelStrip::AmountOfLEDs; ++i) {
			toSet[i] = SignalingStateColor::Off;
			prev[i] = SignalingStateColor::Off;
			ledStrip.SetColor(i, SignalingStateColor::Off);
		}

		ledStrip.UpdateStrip();

	}
};

using Color = NeoPixelStrip::Color;


static StripColorHandler *strip {nullptr};

/**
 *	We are making an effect where the LEDs go from 0 intensity to the default Intensity value and
 *	then in the opposite direction.
 */
void EffectFading(Color color, uint32_t period ) {
	const uint32_t effectTimeMs = millis() % period;
	const uint8_t valueHalfPeriod = (uint8_t)( ( (uint32_t)NeoPixelStrip::DefaultIntensity * effectTimeMs )  / (period/2) );
	const uint8_t intensity = ( effectTimeMs < (period/2) ) ?
			valueHalfPeriod : 										// Rising
			( 2 * NeoPixelStrip::DefaultIntensity - valueHalfPeriod);	// Falling
	strip->SetAll(NeoPixelStrip::Color(color,intensity));
}

/*
 * The LED will be on for 75% of the cycle and then fade off  and blink fast.
 */
void EffectFadeAndBlink(Color color, uint32_t period )
{
	const uint32_t effectTimeMs 		= millis() % period;
	if ((effectTimeMs < period*0.75)
			|| ((effectTimeMs < period*0.85) && (effectTimeMs >= period*0.8))
			|| (effectTimeMs >= period*0.9)) {
		color.intensity = (NeoPixelStrip::DefaultIntensity);
	} else if ( ((effectTimeMs < period*0.8) && (effectTimeMs >= period*0.75))
			|| ((effectTimeMs < period*0.9) && (effectTimeMs >= period*0.85)) ){
		color.intensity = 0;
	}
	strip->SetAll(color);
}

/*
 * In this effect, we will have LEDs on and off alternately. And we will switch the order every half cycle.
 */
void EffectHalfOn(const Color color, const uint32_t period ) {
	const uint32_t 	effectTimeMs = millis() % period;
	const bool 	selectedPair = ( effectTimeMs < ( period / 2 ) ) ;
	for (uint8_t idx = 0 ; idx < NeoPixelStrip::AmountOfLEDs ; ++idx) {
		strip->Set(idx, ( selectedPair ? SignalingStateColor::Off : color ) );
	}
	strip->Update();
}

[[noreturn]] void LedTaskCode(void*) noexcept
{

	delay(LedTaskStartUpDelayMs); // Delay before start

	while(true)
	{
		if(!StepTimer::IsSynced()){
			/*
			 * When it is not sync, we have a basic effect with the "Disconnected" color
			 */
			constexpr uint32_t disconnectedPeriod = 750;
			EffectFading(SignalingStateColor::Disconnected, disconnectedPeriod);
		}
		else if ( stripId == MaxLedStrips ) {
			/*
			 * When it is sync but the LED strip is not configured, we will blink
			 * as the Sync LED.
			 */
			(StepTimer::GetMasterTime() & (1u << 19)) != 0 ?
					strip->SetAll(SignalingStateColor::NotInitialized) :
					strip->SetAll(SignalingStateColor::Error);
		}
		else{
			switch( currentState )
			{
			case LedHandler::SignalingState::Off :
				strip->SetAll(SignalingStateColor::Off);
				break;
			case LedHandler::SignalingState::Error :
				strip->SetAll(SignalingStateColor::Error);
				break;
			case LedHandler::SignalingState::Idle :
				strip->SetAll(SignalingStateColor::Idle);
				break;
			case LedHandler::SignalingState::Selected :
				strip->SetAll(SignalingStateColor::Selected);
				break;
			case LedHandler::SignalingState::Warning :
				{
					const uint32_t warningPeriod = 3000 ; // [ms]
					EffectFadeAndBlink(SignalingStateColor::Warning, warningPeriod );
				}
				break;
			case LedHandler::SignalingState::OutOfFilament :
				{
					const uint32_t outOfFilamentPeriod = 1000 ; // [ms]
					EffectHalfOn(SignalingStateColor::OutOfFilament, outOfFilamentPeriod );
				}
				break;
			case LedHandler::SignalingState::OnTemperature :
				strip->SetAll(SignalingStateColor::OnTemperature);
				break;
			case LedHandler::SignalingState::Heating :
				strip->SetAll(SignalingStateColor::Heating);
				break;
			case LedHandler::SignalingState::Cooling :
				strip->SetAll(SignalingStateColor::Cooling);
				break;
			case LedHandler::SignalingState::Manual:
				{
					NeoPixelStrip::Color color = NeoPixelStrip::Color( userRed , userGreen, userBlue );
					if(userEffectTime == 0){
						// TODO: Add error if case is not 0
						strip->SetAll(color);
					}
					else{
						switch(userEffect){
						case 1:
							EffectFading(color, userEffectTime);
							break;
						case 2:
							EffectFadeAndBlink(color, userEffectTime);
							break;
						case 3:
							EffectHalfOn(color, userEffectTime);
							break;
						default:
							strip->SetAll(color);
						}
					}
				}
				break;
			case LedHandler::SignalingState::MAX:
			default:
				strip->SetAll(SignalingStateColor::Error);
			}
		}
		strip->Update();
		delay(LedTaskLoopDelayMs);
	}
}

void LedHandler::Init() noexcept
{
	strip = new StripColorHandler();

	ledTask = new Task<LedTaskStackWords>;
	ledTask->Create(LedTaskCode, "LED", nullptr, TaskPriority::LedStates);

}

void LedHandler::SetState(LedHandler::SignalingState state){
	currentState = state;
}

GCodeResult LedHandler::HandleM950(CanMessageGenericParser& parser, const StringRef& reply) {
	uint16_t stripNumber;
	if (!parser.GetUintParam('E', stripNumber))
	{
		// Multiple stips not supported!
		reply.copy("Missing strip number parameter in M950Led message");
		return GCodeResult::remoteInternalError;
	}
	stripId = stripNumber;
	if (stripNumber >= MaxLedStrips)
	{
		reply.printf("LED strip number %u is too high for expansion board %u", stripNumber, CanInterface::GetCanAddress());
		return GCodeResult::error;
	}
	LedHandler::SetState(LedHandler::SignalingState::Idle);
	return GCodeResult::ok;
}

GCodeResult LedHandler::HandleM150(CanMessageGenericParser& parser, const StringRef& reply){
	uint16_t stripNumber = 0;								// strip number may be omitted, defaults to 0
	if(stripId == MaxLedStrips){
		reply.copy("Strip not initialized in board %u", CanInterface::GetCanAddress());
		return GCodeResult::remoteInternalError;
	}
	if(!parser.GetUintParam('E', stripNumber)){
		reply.copy("Missing strip number parameter in M150Led message");
		return GCodeResult::remoteInternalError;
	}
	if (stripNumber >= MaxLedStrips)
	{
		reply.printf("LED strip number %u is too high for expansion board %u", stripNumber, CanInterface::GetCanAddress());
		return GCodeResult::error;
	}
	if(stripId != stripNumber){
		reply.printf("Not valid strip number: %u", stripNumber);
		return GCodeResult::remoteInternalError;
	}
	uint8_t state;
	if(!parser.GetUintParam('S', state)){
		reply.copy("Missing state parameter (S) in M150Led message");
		return GCodeResult::remoteInternalError;
	}
	if( state >= static_cast<uint8_t>(LedHandler::SignalingState::MAX) ){
		reply.printf("LED state %u is too high for the strip %u:%u", state, CanInterface::GetCanAddress() , stripNumber);
		return GCodeResult::error;
	}
	if(state == static_cast<uint8_t>(LedHandler::SignalingState::Manual)) {
		uint8_t red{0}, green{0}, blue{0};
		uint8_t effect{0}, effectTime{0};
		(void)parser.GetUintParam('R', red);
		(void)parser.GetUintParam('U', green);
		(void)parser.GetUintParam('B', blue);
		(void)parser.GetUintParam('Y', effect);
		(void)parser.GetUintParam('W', effectTime); // [0.1*sec] for 0.0 to 25.5 sec
		userRed = red;
		userGreen = green;
		userBlue = blue;
		userEffect = effect;
		userEffectTime = (100*(uint32_t)effectTime);
	}
	currentState = static_cast<LedHandler::SignalingState>( state );
	return GCodeResult::ok;
}


#endif

// End
