/*
 * AdcStreamHandler.h
 *
 *  Created on: 14 Jul 2023
 *      Author: BigRep
 */

#include "AdcStreamHandler.h"

#if SUPPORT_ADCSTREAM

#include <RTOSIface/RTOSIface.h>
#include <CanMessageFormats.h>
#include <Platform/Platform.h>
#include <Platform/TaskPriorities.h>
#include <CanMessageBuffer.h>
#include <CAN/CanInterface.h>
#include <CanMessageGenericParser.h>
#include <CanMessageGenericTables.h>
#include <AppNotifyIndices.h>

#define TEST_PACKING	0

constexpr uint16_t DefaultSamplingRate = 400;
constexpr uint8_t DefaultResolution = 16;

constexpr size_t AccelerometerTaskStackWords = 150;
static Task<AccelerometerTaskStackWords> *adcStreamTask = nullptr;


static adcFastStream *stream = nullptr;
static bool present = false;

static volatile uint32_t numSamplesRequested;
static volatile uint8_t axesRequested;
static volatile bool running = false;
static volatile bool successfulStart = false;
static volatile bool failedStart = false;


[[noreturn]] void AdcStreamTaskCode(void*) noexcept
{
	for (;;)
	{
		TaskBase::TakeIndexed(NotifyIndices::AdcStream);
		if (running)
		{
			// Collect and send the samples
			CanMessageBuffer buf;
			CanMessageAccelerometerData& msg = *(buf.SetupRequestMessageNoRid<CanMessageAccelerometerData>(CanInterface::GetCanAddress(), CanInterface::GetCurrentMasterAddress()));
			const unsigned int MaxSamplesInBuffer = msg.SetAxesAndResolution(axesRequested, DefaultResolution);

			unsigned int samplesSent = 0;
			unsigned int samplesRead = 0;
			unsigned int samplesInBuffer = 0;
			unsigned int samplesWanted = numSamplesRequested;
			size_t canDataIndex = 0;
			uint16_t data[MaxSamplesInBuffer] = {0};
			bool overflowed = false;

			if (stream->IsValid())   															//StartCollecting(TranslateAxes(axesRequested)))
			{
				successfulStart = true;
				do
				{

					while(((samplesWanted - samplesRead) > 0) && (samplesRead < MaxSamplesInBuffer))
					{
						if(!stream->GetValues(data[samplesRead]))
						{
							delay(2);
							break;
						}
						++samplesRead;
					}


					while (samplesRead != 0)
					{
						unsigned int samplesToCopy = min<unsigned int>(samplesRead, MaxSamplesInBuffer - samplesInBuffer);
						while (samplesToCopy != 0)
						{

							msg.data[samplesInBuffer] = data[canDataIndex];

							++canDataIndex;
							++samplesInBuffer;
							--samplesToCopy;
							--samplesWanted;
							--samplesRead;
						}


						if (samplesInBuffer == MaxSamplesInBuffer || samplesWanted == 0)
						{
							msg.firstSampleNumber = samplesSent;
							msg.numSamples = samplesInBuffer;
							msg.actualSampleRate = 0;
							msg.overflowed = overflowed;
							msg.lastPacket = (samplesWanted == 0);
							msg.zero = 0;

							buf.dataLength = msg.GetActualDataLength();
							CanInterface::Send(&buf);

							samplesSent += samplesInBuffer;
							samplesInBuffer = 0;
							canDataIndex = 0;
							overflowed = false;
							//bitsUsed = 0;
							//bitsPending = 0;
						}
					}
					canDataIndex = 0;
					samplesRead = 0;

				} while (samplesWanted != 0);
			}
			else
			{
				failedStart = true;
			}

			//TODO: Do we need to stop collecting data?
			//accelerometer->StopCollecting();

			// Wait for another command
			running = false;
		}
	}
}


// Interface functions called by the main task
void AdcStreamHandler::Init(int _stream) noexcept
{
	stream = Platform::GetAdcFilter(_stream);
	present = (stream != nullptr);
	if(adcStreamTask == nullptr)
	{
		adcStreamTask = new Task<AccelerometerTaskStackWords>;
		adcStreamTask->Create(AdcStreamTaskCode, "ADCSTREAM", nullptr, TaskPriority::AdcStream);
	}
}

bool AdcStreamHandler::IsPresent() noexcept
{
	return present;
}

bool AdcStreamHandler::IsRunning() noexcept
{
	return running;
}

// Translate the orientation from a 2-digit number to translation tables, returning true if successful, false if bad orientation
GCodeResult AdcStreamHandler::ProcessConfigRequest(const CanMessageGeneric& msg, const StringRef &reply) noexcept
{
	CanMessageGenericParser parser(msg, M955Params);
	uint8_t deviceNumber;
	if (!parser.GetUintParam('P', deviceNumber))
	{
		reply.copy("Bad M955 message");
		return GCodeResult::error;
	}
	//TODO: Allow more then one stream at a time
	if (!present)
	{
		reply.printf("Analog stream %u.%u not present (nullpointer)", CanInterface::GetCanAddress(), deviceNumber);
		return GCodeResult::error;
	}

	if (running)
	{
		reply.printf("Analog stream %u.%u is busy collecting data", CanInterface::GetCanAddress(), deviceNumber);
		return GCodeResult::error;
	}

	uint8_t localOrientation;
	if (parser.GetUintParam('I', localOrientation))
	{
		reply.copy("Invalid orientation");
		return GCodeResult::error;
	}

	reply.printf("Adc stream samples at %uHz with %u-bit resolution",DefaultSamplingRate, DefaultResolution);
	return GCodeResult::ok;


}

GCodeResult AdcStreamHandler::ProcessStartRequest(const CanMessageStartAccelerometer& msg, const StringRef& reply) noexcept
{
	if (stream == nullptr)
	{
		reply.printf("Analog stream %u.%u not present", CanInterface::GetCanAddress(), msg.deviceNumber);
		return GCodeResult::error;
	}

	if (running)
	{
		reply.printf("Analog stream %u.%u is busy collecting data", CanInterface::GetCanAddress(), msg.deviceNumber);
		return GCodeResult::error;
	}

	axesRequested = msg.axes;
	numSamplesRequested = msg.numSamples;
	successfulStart = false;
	failedStart = false;
	running = true;
	delay(10); // Small delay before starting

	adcStreamTask->Give(NotifyIndices::AdcStream);

	const uint32_t startTime = millis();
	do
	{
		delay(5);
		if (successfulStart)
		{
			return GCodeResult::ok;
		}
	} while (!failedStart && millis() - startTime < 1000);

	reply.copy("Failed to start Analog stream data collection");

	return GCodeResult::error;
}

void AdcStreamHandler::Diagnostics(const StringRef& reply) noexcept
{
	//TODO: Implement Diagnostics
	reply.lcatf("Adc stream samples at %uHz with %u-bit resolution",DefaultSamplingRate, DefaultResolution);
}

#endif

// End
