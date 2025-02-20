/*
 * ADXL345.h
 *
 *  Created on: 10 Jan 2022
 *      Author: BigRep
 */

#ifndef SRC_HARDWARE_ADXL345_H_
#define SRC_HARDWARE_ADXL345_H_

#include <RepRapFirmware.h>

#if SUPPORT_ADXL345

#include "SharedI2CClient.h"

class ADXL345 : public SharedI2CClient
{
public:
	ADXL345(SharedI2CMaster& dev, Pin p_int1Pin) noexcept;

	// Do a quick test to check whether the accelerometer is present, returning true if it is
	bool CheckPresent() noexcept;

	// Return the type name of the accelerometer.
	const char *GetTypeName() const noexcept { return "ADXL345"; } ;

	// Configure the accelerometer to collect at or near the requested sampling rate and the requested resolution in bits.
	bool Configure(uint16_t& samplingRate, uint8_t& resolution) noexcept;

	// Start collecting data
	bool StartCollecting(uint8_t axes) noexcept;

	// Collect some data from the FIFO, suspending until the data is available
	unsigned int CollectData(const uint16_t **collectedData, uint16_t &dataRate, bool &overflowed) noexcept;

	// Stop collecting data
	void StopCollecting() noexcept;

	// Get a status byte
	uint8_t ReadStatus() noexcept;

	// Used by the ISR
	void Int1Isr() noexcept;

	// Used by diagnostics
	bool HasInterruptError() const noexcept { return false; }

private:
	enum class ADXLRegister : uint8_t
	{
		DeviceID =					0x00,		// Device ID
		TapThreshold =				0x1D,		// Tap Threshold.
		OffsetX	=					0x1E,		// X-Axis Offset.
		OffsetY =					0x1F,		// Y-Axis Offset.
		OffsetZ =					0x20,		// Z- Axis Offset.
		TapDuration =				0x21,		// Tap Duration.
		TapLatency =				0x22,		// Tap Latency.
		TapWindow =					0x23,		// Tap Window.
		ActivityThreshold =			0x24,		// Activity Threshold
		InactivityThreshold =		0x25,		// Inactivity Threshold
		InactivityTime =			0x26,		// Inactivity Time
		Control =					0x27,		// Axis Enable Control for Activity and Inactivity Detection
		FreeFallThreshold =			0x28,		// Free-Fall Threshold.
		FreeFallTime =				0x29,		// Free-Fall Time.
		TapAxes =					0x2A,		// Axis Control for Tap/Double Tap.
		TapStatus =					0x2B,		// Source of Tap/Double Tap
		DataRateControl =			0x2C,		// Data Rate and Power mode Control
		PowerControl =				0x2D,		// Power-Saving Features Control
		InterruptEnableControl =	0x2E,		// Interrupt Enable Control
		InterruptMappingControl	=	0x2F,		// Interrupt Mapping Control
		InterruptSources =			0x30,		// Source of Interrupts
		DataFormatControl =			0x31,		// Data Format Control
		DataX0 =					0x32,		// X-Axis Data 0
		DataX1 =					0x33,		// X-Axis Data 1
		DataY0 =					0x34,		// Y-Axis Data 0
		DataY1 =					0x35,		// Y-Axis Data 1
		DataZ0 =					0x36,		// Z-Axis Data 0
		DataZ1 =					0x37,		// Z-Axis Data 1
		FIFOControl =				0x38,		// FIFO Control
		FIFOStatus =				0x39,		// FIFO Status
	};

	bool ReadRegisters(ADXLRegister reg, size_t numToRead, uint8_t offset = 0) noexcept;
	bool WriteRegisters(ADXLRegister reg, size_t numToWrite) noexcept;
	bool ReadRegister(ADXLRegister reg, uint8_t& val) noexcept;
	bool WriteRegister(ADXLRegister reg, uint8_t val) noexcept;

	volatile TaskHandle taskWaiting;
	uint32_t 			firstInterruptTime = 0;
	uint32_t 			lastInterruptTime = 0;
	uint32_t 			totalNumRead = 0;
	Pin 				int1Pin;

	alignas(2) struct
	{
		uint8_t dummy;
		uint8_t reg;
		uint8_t data[6 * 35];				// There is an extra entry in the output buffer
	} transferBuffer;
};

#endif

#endif /* SRC_HARDWARE_ADXL345_H_ */
