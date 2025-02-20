/*
 * LedHandler.h
 *
 *  Created on: 01.08.2022
 *      Author: BigRep
 */

#ifndef SRC_LEDSTRIPS_STATES_NEOPIXELSTRIP_H_
#define SRC_LEDSTRIPS_STATES_NEOPIXELSTRIP_H_

#include <RepRapFirmware.h>

#if SUPPORT_LED_STATES

class NeoPixelStrip
{
public:
	static constexpr size_t  AmountOfLEDs = 6;			// Amount of LEDs in the Strip
	static constexpr uint8_t DefaultIntensity = 100;	// Default intensity
	static constexpr uint32_t SpiFreq = 800000 * 3;		// SPI frequency. The LEDs work with 800KHz but we need 3 bits to create a single color-bit.

	struct Color{
		uint8_t red {0}, green{0}, blue{0};
		uint8_t intensity 	{DefaultIntensity};

		Color() = default;
		~Color() = default;
		Color(const Color&) = default;
		Color& operator=(const Color&) = default;

		Color(uint8_t _red, uint8_t _green,uint8_t _blue, uint8_t _intensity = DefaultIntensity) :
			red(_red), green(_green), blue(_blue), intensity((_intensity) > 100 ? 100 : _intensity) { };

		Color(const uint32_t hexColor, const uint8_t _intensity = DefaultIntensity) :
			red((uint8_t)((hexColor >> 16) & 0x0FF)), green((uint8_t)((hexColor >> 8) & 0x0FF)),
			blue((uint8_t)((hexColor >> 0) & 0x0FF)), intensity( (_intensity) > 100 ? 100 : _intensity ) { };

		Color(const Color& color, const uint8_t _intensity) :
			red(color.red), green(color.green), blue(color.blue), intensity((_intensity) > 100 ? 100 : _intensity) { };

		uint8_t GetOutputRed() 	const noexcept { return (uint8_t)(( (uint16_t)red   * (uint16_t)intensity) / (uint16_t)255 );};
		uint8_t GetOutputGreen()const noexcept { return (uint8_t)(( (uint16_t)green * (uint16_t)intensity) / (uint16_t)255 );};
		uint8_t GetOutputBlue()	const noexcept { return (uint8_t)(( (uint16_t)blue  * (uint16_t)intensity) / (uint16_t)255 );};
	};

	void Init() noexcept;

	void UpdateStrip() noexcept;

	void SetColor(const size_t ledID,const Color &color) noexcept
	{
		if( ledID < AmountOfLEDs )
		{
			SetColor((uint8_t)ledID, color.GetOutputRed(), color.GetOutputGreen(), color.GetOutputBlue());
		}
	};

private:

	void AppendBit(const uint8_t led, const uint8_t bitNumber, const uint8_t color, const uint8_t value) noexcept;
	void SetColor(const uint8_t ledID, const uint8_t r, const uint8_t g,const uint8_t b) noexcept;
};

bool operator==(const NeoPixelStrip::Color &lhs, const NeoPixelStrip::Color &rhs);
bool operator!=(const NeoPixelStrip::Color &lhs, const NeoPixelStrip::Color &rhs);

namespace NeoPixelColors{
	const auto Red   	= NeoPixelStrip::Color( 255,   0,   0);
	const auto Green   	= NeoPixelStrip::Color(   0, 255,   0);
	const auto Blue   	= NeoPixelStrip::Color(   0,   0, 255);
	const auto Yellow  	= NeoPixelStrip::Color( 127, 127,   0);
	const auto Cyan  	= NeoPixelStrip::Color(   0, 127, 127);
	const auto Purple  	= NeoPixelStrip::Color( 127,   0, 127);
	const auto White  	= NeoPixelStrip::Color(  87,  87,  87);
	const auto Black  	= NeoPixelStrip::Color(   0,   0,   0);
	const auto Orange  	= NeoPixelStrip::Color( 200,  55,   0);
};

#endif

#endif /* SRC_LEDSTRIPS_STATES_NEOPIXELSTRIP_H_ */
