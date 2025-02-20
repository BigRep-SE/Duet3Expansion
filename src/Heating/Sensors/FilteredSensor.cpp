/*
 * FilteredSensor.cpp
 *
 *  Created on: 04.12.2022
 *      Author: BigRep
 */

#include "FilteredSensor.h"

#include <Platform/Platform.h>
#include "CanMessageGenericParser.h"

GCodeResult FilteredSensor::ConfigureFilter(const CanMessageGenericParser& parser, bool &changed)
{
	changed = false;
	if (parser.GetFloatParam('C', kIIR))
	{
		changed = true;
	}
	return GCodeResult::ok;
}


void FilteredSensor::SetK(float k) noexcept
{
	if(k > MaxK) {
		k = 1.0f; // No filter
	}
	else if( k < MinK)
	{
		k = MinK;
	}
	kIIR = k;
}
