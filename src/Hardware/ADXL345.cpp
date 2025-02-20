/*
 * ADXL345.cpp
 *
 *  Created on: 10 Jan 2022
 *      Author: BigRep
 */

#include "ADXL345.h"
#include <AppNotifyIndices.h>

#if SUPPORT_ADXL345

#include <Hardware/IoPorts.h>
#include <Movement/StepTimer.h>

#ifndef SUPPORT_ADXL345_ALT_ADDR
constexpr uint16_t ADXL345Address = 0x53;			// ALT ADDRESS pin ground in the schematic
#else
constexpr uint16_t ADXL345Address = 0x1D;			// ALT ADDRESS pin high in the schematic
#endif

constexpr uint8_t  FifoInterruptLevel = 8;			// how full the FIFO must get before we want an interrupt

static constexpr uint8_t WhoAmIValue = 0xE5;		// Reset value for ADXLRegister::DeviceID

constexpr uint32_t ADXL345I2CTimeout = 25;

ADXL345::ADXL345(SharedI2CMaster& dev, Pin p_int1Pin) noexcept
	: SharedI2CClient(dev, ADXL345Address), taskWaiting(nullptr), int1Pin(p_int1Pin)
{
}

// Do a quick test to check whether the accelerometer is present, returning true if it is
bool ADXL345::CheckPresent() noexcept
{
	uint8_t val;
	bool ok = ReadRegister(ADXLRegister::DeviceID, val);

	return ok && val == WhoAmIValue;
}

uint8_t ADXL345::ReadStatus() noexcept
{
	uint8_t val;
	return (ReadRegister(ADXLRegister::FIFOStatus, val)) ? val : 0xFF;
}

// Configure the accelerometer to collect for the requested axis at or near the requested sampling rate and the requested resolution in bits.
// Update the sampling rate and resolution to the actual values used.
bool ADXL345::Configure(uint16_t& samplingRate, uint8_t& resolution) noexcept
{
	if(resolution != 10)
	{
		// For 2g the resolution is 10.
		resolution = 10;
	}

	uint8_t odr;  // Output data rate
	// Limitation via I2C.
	// TODO: Test if we can go faster.
	if(samplingRate >= 800)
	{
		odr = 0x0D;
		samplingRate = 800;
	}
	else if(samplingRate >= 400)
	{
		odr = 0x0C;
		samplingRate = 400;
	}
	else if(samplingRate >= 200)
	{
		odr = 0x0B;
		samplingRate = 200;
	}
	else
	{
		odr = 0x0A;
		samplingRate = 100;
	}

	bool ok = WriteRegister(ADXLRegister::DataRateControl, odr);
	if (ok)
	{
		// Set the fifo mode
		// Mode 2: Stream mode
		ok = WriteRegister(ADXLRegister::FIFOControl, (2u << 6) | (FifoInterruptLevel - 1));
		ok = WriteRegister(ADXLRegister::DataFormatControl, 0x04); // Justify left
	}
	return ok;
}

void Int1Interrupt(CallbackParameter p) noexcept;						// forward declaration

// Start collecting data, returning true if successful
bool ADXL345:: StartCollecting(uint8_t axes) noexcept
{
	// Clear the fifo
	uint8_t val = ReadStatus();
	bool ok = WriteRegister(ADXLRegister::PowerControl, 0x08);			// Measure
	while ( (val & 0x3F) != 0)		// while fifo not empty
	{
		ReadRegisters(ADXLRegister::DataX0, 6);
		val = ReadStatus();
	}

	// The experimental setup needs the pullup resistor enabled on the interrupt pin
	pinMode(int1Pin, INPUT_PULLUP);

	totalNumRead = 0;

	ok &= WriteRegister(ADXLRegister::InterruptEnableControl, 0x02);  	// Watermark

	return ok && AttachPinInterrupt(int1Pin, Int1Interrupt, InterruptMode::rising, CallbackParameter(this));
}

// Collect some 8-bit data from the FIFO, suspending until the data is available
unsigned int ADXL345::CollectData(const uint16_t **collectedData, uint16_t &dataRate, bool &overflowed) noexcept
{
	// Wait until we have some data
	taskWaiting = TaskBase::GetCallerTaskHandle();
	while (!digitalRead(int1Pin))
	{
		TaskBase::TakeIndexed(NotifyIndices::AccelerometerHardware);
	}
	taskWaiting = nullptr;

	// Get the fifo status to see how much data we can read and whether the fifo overflowed
	uint8_t fifoStatus;
	if (!ReadRegister(ADXLRegister::FIFOStatus, fifoStatus))
	{
		return 0;
	}

	uint8_t numToRead = fifoStatus & 0x3F;

	if (numToRead != 0)
	{
		// Read the data
		// When the auto-increment bit is set in the register number, after reading register 0x2D it wraps back to 0x28
		// The datasheet doesn't mention this but ST app note AN3308 does
		for(int i = 0; i < numToRead; i++)
		{
			if (!ReadRegisters(ADXLRegister::DataX0, 6, i*6))
			{
				return 0;
			}
		}

		*collectedData = reinterpret_cast<const uint16_t*>(transferBuffer.data);
		//overflowed = (fifoStatus & 0x40) != 0;
		// TODO: Check overflow!
		dataRate = (totalNumRead == 0) ? 0 : (totalNumRead * StepTimer::StepClockRate)/(lastInterruptTime - firstInterruptTime);
		totalNumRead += numToRead;
	}
	return numToRead;
}

// Stop collecting data
void ADXL345::StopCollecting() noexcept
{
	// TODO: Send it to Sleep with the Sleep bit.
	WriteRegister(ADXLRegister::InterruptEnableControl, 0x00);  	// Disable interruptions Watermark
	WriteRegister(ADXLRegister::PowerControl, 0);

}

bool ADXL345::ReadRegisters(ADXLRegister reg, size_t numToRead, uint8_t offset) noexcept
{
	transferBuffer.reg = (uint8_t)reg;
	return Transfer(&transferBuffer.reg, transferBuffer.data+offset, 1, numToRead, ADXL345I2CTimeout);
}

bool ADXL345::WriteRegisters(ADXLRegister reg, size_t numToWrite) noexcept
{
	if ((uint8_t)reg < 0x1E)						// don't overwrite the factory calibration values
	{
		return false;
	}

	transferBuffer.reg = (uint8_t)reg;
	return Transfer(&transferBuffer.reg, transferBuffer.data, 1 + numToWrite, 0, ADXL345I2CTimeout);
}

bool ADXL345::ReadRegister(ADXLRegister reg, uint8_t& val) noexcept
{
	const bool ret = ReadRegisters(reg, 1);
	if (ret)
	{
		val = transferBuffer.data[0];
	}
	return ret;
}

bool ADXL345::WriteRegister(ADXLRegister reg, uint8_t val) noexcept
{
	transferBuffer.data[0] = val;
	return WriteRegisters(reg, 1);
}

void ADXL345::Int1Isr() noexcept
{
	const uint32_t now = StepTimer::GetTimerTicks();
	if (totalNumRead == 0)
	{
		firstInterruptTime = now;
	}
	lastInterruptTime = now;
	TaskBase::GiveFromISR(taskWaiting, NotifyIndices::AccelerometerHardware);
	taskWaiting = nullptr;
}

void Int1Interrupt(CallbackParameter p) noexcept
{
	static_cast<ADXL345*>(p.vp)->Int1Isr();
}

#endif

// End
