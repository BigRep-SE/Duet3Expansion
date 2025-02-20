/*
 * AveragingFilter.h
 *
 *  Created on: 7 Sep 2018
 *      Author: David
 */

#ifndef SRC_PLATFORM_AVERAGINGFILTER_H_
#define SRC_PLATFORM_AVERAGINGFILTER_H_

#include "RepRapFirmware.h"
#include "RTOSIface/RTOSIface.h"

// Class to perform averaging of values read from the ADC
// numAveraged should be a power of 2 for best efficiency
template<size_t numAveraged> class AveragingFilter
{
public:
#if SUPPORT_ANALOG_THRESHOLD
	typedef void (*FilteredCallbackFunction)(CallbackParameter,uint32_t);
	constexpr inline static size_t MAX_CALLBACKS = 2;
#endif

	AveragingFilter() noexcept
	{
		Init(0);
#ifdef SUPPORT_ADCSTREAM
		for(auto &callback : filteredCallbacks){
			callback = nullptr;
		}
#endif
	}

	void Init(uint16_t val) volatile noexcept
	{
		TaskCriticalSectionLocker lock;

		sum = (uint32_t)val * (uint32_t)numAveraged;
		index = 0;
		isValid = false;
#ifdef SUPPORT_ADCSTREAM
		tail = 0;
		size = 0;
#endif
		for (size_t i = 0; i < numAveraged; ++i)
		{
			readings[i] = val;
		}
	}

	// Call this to put a new reading into the filter
	void ProcessReading(uint16_t r) noexcept
	{
#if SUPPORT_ANALOG_THRESHOLD
		uint32_t value;
		bool toCall = false;
#endif
		{
			TaskCriticalSectionLocker lock;
			sum = sum - readings[index] + r;
			readings[index] = r;

#ifdef SUPPORT_ADCSTREAM
			// if new values are available, move the reading index forward
			if (index == tail) {
				tail = (tail + 1) % numAveraged;
			}
			if (size < numAveraged) {
				++size;
			}
#endif

			++index;
			if (index == numAveraged)
			{
				index = 0;
				isValid = true;
#if SUPPORT_ANALOG_THRESHOLD
				value = sum/numAveraged;
				toCall = true;
			}
		}
		if( toCall ) {
			for(size_t i = 0; i < MAX_CALLBACKS ; ++i ) {
				if(filteredCallbacks[i]) {
					filteredCallbacks[i](filteredParameters[i],value);
				}
#endif
			}
		}
	}

#ifdef SUPPORT_ADCSTREAM
	// Return a single value from the end of the queue
	size_t GetValues(uint16_t &value) noexcept
	{
		TaskCriticalSectionLocker lock;

		if (size > 0) {
			value = readings[tail]+0x8000;
			tail = (tail + 1) % numAveraged;
			--size;
			return true; 
		}
		return false; // no more values to read - the buffer has been completely read
	}
#endif

	// Return the raw sum
	uint32_t GetSum() const volatile noexcept
	{
		return sum;
	}

	// Return true if we have a valid average
	bool IsValid() const volatile noexcept
	{
		return isValid;
	}

	// Get the latest reading
	uint16_t GetLatestReading() const volatile noexcept
	{
		size_t indexOfLastReading = index;			// capture volatile variable
		indexOfLastReading = (indexOfLastReading == 0) ? numAveraged - 1 : indexOfLastReading - 1;
		return readings[indexOfLastReading];
	}

#if SUPPORT_ANALOG_THRESHOLD
	bool SetCallbackFiltered( FilteredCallbackFunction fn, CallbackParameter param ) {
		// Saving the callbacks
		for(size_t i = 0; i < MAX_CALLBACKS ; ++i) {
			if(filteredCallbacks[i] == nullptr || filteredCallbacks[i] == fn) {
				filteredParameters[i] = param;
				filteredCallbacks[i]  = fn;
				return true;
			}
		}
		return false;
	}
#endif

	static constexpr size_t NumAveraged() noexcept { return numAveraged; }

	// Function used as an ADC callback to feed a result into an averaging filter
	static void CallbackFeedIntoFilter(CallbackParameter cp, uint32_t val) noexcept;

	bool CheckIntegrity() const noexcept;

private:
	uint16_t readings[numAveraged];
	size_t index;
	uint32_t sum;
	bool isValid;

#ifdef SUPPORT_ADCSTREAM
	size_t tail;  // index of the next oldest value to read
	size_t size;  // number of values to read
#endif	

#if SUPPORT_ANALOG_THRESHOLD
	FilteredCallbackFunction filteredCallbacks[MAX_CALLBACKS];
	CallbackParameter filteredParameters[MAX_CALLBACKS];
#endif

	//invariant(sum == + over readings)
	//invariant(index < numAveraged)
};

// This is called from an ISR or high priority task to add a new reading to the filter.
template<size_t numAveraged> void AveragingFilter<numAveraged>::CallbackFeedIntoFilter(CallbackParameter cp, uint32_t val) noexcept
{
	static_cast<AveragingFilter<numAveraged>*>(cp.vp)->ProcessReading((uint16_t)val);
}

template<size_t numAveraged> bool AveragingFilter<numAveraged>::CheckIntegrity() const noexcept
{
	AtomicCriticalSectionLocker lock;

	uint32_t locSum = 0;
	for (size_t i = 0; i < numAveraged; ++i)
	{
		locSum += readings[i];
	}
	return locSum == sum;
}

#endif /* SRC_PLATFORM_AVERAGINGFILTER_H_ */
