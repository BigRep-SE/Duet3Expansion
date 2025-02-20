/*
 * LedHandler.cpp
 *
 *  Created on: 01.08.2022
 *      Author: BigRep
 */

#include "NeoPixelStrip.h"

#if SUPPORT_LED_STATES

#include <RTOSIface/RTOSIface.h>
#include <CanMessageFormats.h>
#include <Platform/Platform.h>
#include <CanMessageBuffer.h>
#include <CAN/CanInterface.h>
#include <CanMessageGenericParser.h>
#include <CanMessageGenericTables.h>

# include <hri_sercom_e54.h>
#include "DmacManager.h"
#include "Serial.h"
#include <Cache.h>

alignas(4) volatile uint8_t ledData[(24 * NeoPixelStrip::AmountOfLEDs * 3)/8];

void NeoPixelStrip::UpdateStrip() noexcept
{
	DmacManager::DisableChannel(DmacChanLedStrip);

	Cache::FlushBeforeDMASend(ledData, ARRAY_SIZE(ledData));

	SERCOM_LEDSTRIP->SPI.CTRLA.bit.ENABLE = 0;
	while (SERCOM_LEDSTRIP->SPI.SYNCBUSY.bit.ENABLE) { }
	DmacManager::EnableChannel(DmacChanLedStrip, DmacPrioLedStrip);
	SERCOM_LEDSTRIP->SPI.CTRLA.bit.ENABLE = 1;
	while (SERCOM_LEDSTRIP->SPI.SYNCBUSY.bit.ENABLE) { }
}

bool operator==(const NeoPixelStrip::Color &lhs, const NeoPixelStrip::Color &rhs){
		return (lhs.red == rhs.red) && (lhs.green == rhs.green) && (lhs.blue == rhs.blue) && (lhs.intensity == rhs.intensity);
};

bool operator!=(const NeoPixelStrip::Color &lhs, const NeoPixelStrip::Color &rhs){
	return !operator==(lhs,rhs);
};


void NeoPixelStrip::AppendBit(const uint8_t led, const uint8_t bitNumber, const uint8_t color, const uint8_t value) noexcept
{
	uint16_t bitToSendPosition =  (uint16_t)3 * ( (uint16_t)((uint16_t)24*(uint16_t)led) + (uint16_t)8*(uint16_t)color + (uint16_t)((uint16_t)7-(uint16_t)bitNumber) );
	uint16_t idx = bitToSendPosition / 8;
	uint16_t offset = bitToSendPosition % 8;

	ledData[idx] &= ~(0x80 >> offset);
	++offset;
	if(offset == 8) {
		offset = 0;
		++idx;
	}
	if(value != 0) {
		ledData[idx] &= ~(0x80 >> offset);
	}
	else {
		ledData[idx] |= 0x80 >> offset;
	}
	++offset;
	if(offset == 8) {
		offset = 0;
		++idx;
	}
	ledData[idx] |= 0x80 >> offset;
}


void NeoPixelStrip::SetColor(const uint8_t led, const uint8_t r, const uint8_t g, const uint8_t b) noexcept
{
	uint8_t color = g;
	for(uint8_t bitToSend = 0; bitToSend < 8; ++bitToSend) {
		uint8_t value = (color & 0x01);
		color = (color >> 1);
		AppendBit(led, bitToSend, 0, value );
	}
	color = r;
	for(uint8_t bitToSend = 0; bitToSend < 8; ++bitToSend) {
		uint8_t value = (color & 0x01);
		color = (color >> 1);
		AppendBit(led, bitToSend, 1, value );
	}
	color = b;
	for(uint8_t bitToSend = 0; bitToSend < 8; ++bitToSend) {
		uint8_t value = (color & 0x01);
		color = (color >> 1);
		AppendBit(led, bitToSend, 2, value );
	}
}

void NeoPixelStrip::Init() noexcept
{
	// The DPLL should be on, as it is used for the TMC-Drivers

	// Set the pin to control the Strip as MOSI (COPI)
	SetPinFunction(LedStripPin, LedStripPinPeriphMode);
	// MISO and CLK are not used so they don't need to be configured.

	// Set the baudrate
	Serial::EnableSercomClock(LedStripSercomNumber);

	// Set up the SERCOM
	const uint32_t regCtrlA = SERCOM_SPI_CTRLA_MODE(3) |
			SERCOM_SPI_CTRLA_DIPO(2) | 						// SPI_RXPO PAD[2] We don't use it
			SERCOM_SPI_CTRLA_DOPO(0) | 						// SPI_TXPO PAD[]
			SERCOM_SPI_CTRLA_FORM(0);						// AMODE: Disable
	const uint32_t regCtrlB = 0;							// 8 bits, slave select disabled, receiver disabled for now
	const uint32_t regCtrlC = 0;							// not 32-bit mode

	if (!hri_sercomspi_is_syncing(SERCOM_LEDSTRIP, SERCOM_SPI_SYNCBUSY_SWRST))
	{
		const uint32_t mode = regCtrlA & SERCOM_SPI_CTRLA_MODE_Msk;
		if (hri_sercomspi_get_CTRLA_reg(SERCOM_LEDSTRIP, SERCOM_SPI_CTRLA_ENABLE))
		{
			hri_sercomspi_clear_CTRLA_ENABLE_bit(SERCOM_LEDSTRIP);
			hri_sercomspi_wait_for_sync(SERCOM_LEDSTRIP, SERCOM_SPI_SYNCBUSY_ENABLE);
		}
		hri_sercomspi_write_CTRLA_reg(SERCOM_LEDSTRIP, SERCOM_SPI_CTRLA_SWRST | mode);
	}
	hri_sercomspi_wait_for_sync(SERCOM_LEDSTRIP, SERCOM_SPI_SYNCBUSY_SWRST);

	hri_sercomspi_write_CTRLA_reg(SERCOM_LEDSTRIP, regCtrlA);
	hri_sercomspi_write_CTRLB_reg(SERCOM_LEDSTRIP, regCtrlB);
	hri_sercomspi_write_CTRLC_reg(SERCOM_LEDSTRIP, regCtrlC);

	hri_sercomspi_write_BAUD_reg(SERCOM_LEDSTRIP, SERCOM_SPI_BAUD_BAUD((uint8_t)( Serial::SercomFastGclkFreq / (2 * SpiFreq) )-1));

	// Set up the DMA descriptors
	// We use separate write-back descriptors, so we only need to set this up once
	DmacManager::SetBtctrl(DmacChanLedStrip,
			DMAC_BTCTRL_STEPSEL_SRC |   	// Step size settings apply to the source address
			DMAC_BTCTRL_SRCINC |			// Source address incrementation is enabled
			DMAC_BTCTRL_EVOSEL_DISABLE |   	// Event generation disabled
			DMAC_BTCTRL_BEATSIZE_BYTE | 	// 8-bit bus transfer
			DMAC_BTCTRL_STEPSIZE_X1 |		// Stepx1
			DMAC_BTCTRL_VALID) ;

	DmacManager::SetSourceAddress(DmacChanLedStrip, ledData);
	DmacManager::SetDestinationAddress(DmacChanLedStrip, &(SERCOM_LEDSTRIP->SPI.DATA.reg));
	DmacManager::SetDataLength(DmacChanLedStrip, ARRAY_SIZE(ledData));
	DmacManager::SetTriggerSourceSercomTx(DmacChanLedStrip, LedStripSercomNumber);
}


#endif

// End
