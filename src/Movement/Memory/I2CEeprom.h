/*
 * I2CEeprom.h
 *
 *  Created on: 20.10.2023
 *      Author: BigRep
 */

#ifndef SRC_MOVEMENT_MEMORY_I2CEEPROM_H_
#define SRC_MOVEMENT_MEMORY_I2CEEPROM_H_

#include <RepRapFirmware.h>

#if SUPPORT_I2C_SENSORS && SUPPORT_EEPROM_MEMORY

#include <Platform/Platform.h>
#include <Hardware/SharedI2CClient.h>

class I2CEeprom : public SharedI2CClient
{
public:
	inline static constexpr size_t MEMORY_SIZE = 512	;

	I2CEeprom(uint8_t addressInput, Pin pinWriteProtect, bool complementLogic = false);
	bool Init();
	bool IsInit() const noexcept { return isInit; };
	bool Read( uint16_t addr, uint8_t *arr, uint8_t length );
	bool Write(uint16_t addr, const uint8_t *arr, uint8_t length );
private:
    uint8_t i2cAddress;
    Pin writeProtect;
    bool complement ;	///< It is recommend to use when there are many zeros.
    static inline constexpr uint32_t I2C_TIMEOUT 	= 50;
    static inline constexpr uint8_t ADDRESS_BASE 	= 0x50;
    static inline constexpr uint8_t WRITE_DELAY 	= 20;			///< [ms] Waiting until the write operation is done
    static inline constexpr size_t 	BUFFER_WRITE_LENGTH 	= 20;

    bool isInit { false };
    // The A8 bit affects the I2C address
    void SetSlaveAddressFromTargetMemoryAddress( uint16_t targetAddress );
    alignas(2) uint8_t bufferWrite[BUFFER_WRITE_LENGTH];

};

#endif // SUPPORT_I2C_SENSORS && SUPPORT_EEPROM_MEMORY

#endif /* SRC_MOVEMENT_MEMORY_I2CEEPROM_H_ */
