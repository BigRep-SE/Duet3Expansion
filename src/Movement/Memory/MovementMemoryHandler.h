/*
 * MovementMemoryHandler.h
 *
 *  Created on: 20.10.2023
 *      Author: BigRep
 */

#ifndef SRC_MOVEMENT_MEMORY_MOVEMENTMEMORYHANDLER_H_
#define SRC_MOVEMENT_MEMORY_MOVEMENTMEMORYHANDLER_H_

#include <RepRapFirmware.h>

#if SUPPORT_I2C_SENSORS && SUPPORT_EEPROM_MEMORY

namespace MovementMemoryHandler {
	void Init( uint32_t _angleRotationRange = 1024 ) noexcept;
	bool IsInit() noexcept;
	uint32_t GetForwardMeters() noexcept;
	uint32_t GetBackwardMeters() noexcept;
}

#endif // SUPPORT_I2C_SENSORS && SUPPORT_EEPROM_MEMORY

#endif /* SRC_FILAMENTMONITORS_MEMORY_FILAMENTMEMORYHANDLER_H_ */
