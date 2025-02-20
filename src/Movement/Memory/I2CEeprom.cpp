/*
 * I2CEeprom.cpp
 *
 *  Created on: 20.10.2023
 *      Author: BigRep
 */

#include <Movement/Memory/I2CEeprom.h>

#if SUPPORT_I2C_SENSORS && SUPPORT_EEPROM_MEMORY

#if (SUPPORT_LIS3DH || SUPPORT_ADXL345)
#include "CommandProcessing/AccelerometerHandler.h"
#endif

I2CEeprom::I2CEeprom(uint8_t addressInput , Pin pinWriteProtect, bool complementLogic) :
	SharedI2CClient(Platform::GetSharedI2C(), ADDRESS_BASE + (addressInput << 1)),
	i2cAddress(ADDRESS_BASE + (addressInput << 1)), writeProtect(pinWriteProtect),
	complement( complementLogic ) {
	// Disable the write protection
	IoPort::SetPinMode(writeProtect, OUTPUT_HIGH);
}

bool I2CEeprom::Init() noexcept {
	uint8_t dataRead ;
	// Dummy Read
	if (Transfer(0, &dataRead, 1, 1, I2C_TIMEOUT))
	{
		isInit = true;
	}
	return isInit;
}

bool I2CEeprom::Read( uint16_t addr, uint8_t *arr, uint8_t length ) {
	SetSlaveAddressFromTargetMemoryAddress(addr);

	delay(1);
	/*
	bool result = Transfer((uint8_t)(addr & 0x00FF), arr, 1, length, I2C_TIMEOUT) ;
	*/
	bool result = true;
	for(uint16_t i = 0 ; i < length ; ++i){
		uint8_t val;
		uint8_t addrByte = (uint8_t)(addr & 0x00FF);
		SetSlaveAddressFromTargetMemoryAddress(addr);
        result &= Transfer(&addrByte, &val, 1, 1, I2C_TIMEOUT);
        addr++;
		delay(1); // Making sure the value is updated
		arr[i] = val;
	}

	if( result && complement ) {
		for( auto i = 0; i < length; i++ ) {
			arr[i] ^= 0xFF;
		}
	}
	delay(1);
	return result;
}


bool I2CEeprom::Write(uint16_t addr, const uint8_t *arr, uint8_t length ) {
	if( length >= BUFFER_WRITE_LENGTH ){
		return false;
	}
	// Enable the write protection
	IoPort::WriteDigital(writeProtect, false);
	/*
	SetSlaveAddressFromTargetMemoryAddress(addr);
	delay(1);
	memset( bufferWrite, 0xFF , length );
	bool result = Transfer((uint8_t)(addr & 0x00FF), (uint8_t*)bufferWrite, (size_t)(length + 1), 0, I2C_TIMEOUT);
	delay(WRITE_DELAY); // Making sure the value is updated
	if(!result){
		return false;
	}
	*/
	memcpy( bufferWrite, arr , length );
	if( complement ) {
		for( auto i = 0; i < length; i++ ){
			bufferWrite[i] ^= 0xFF;
		}
	}
	bool result = true;
	for(uint16_t i = 0 ; i < length ; ++i){
		uint8_t val = bufferWrite[i];
		uint8_t addrByte = (uint8_t)(addr & 0x00FF);
		SetSlaveAddressFromTargetMemoryAddress(addr);
		result &= Transfer(&addrByte, &val, 2, 0, I2C_TIMEOUT);
		addr++;
		delay(WRITE_DELAY); // Making sure the value is updated
	}

	/*
	result = Transfer((uint8_t)(addr & 0x00FF), (uint8_t*)bufferWrite, (size_t)(length + 1), 0, I2C_TIMEOUT);
	delay(WRITE_DELAY); // Making sure the value is updated
	*/
	// Disable the write protection
	IoPort::WriteDigital(writeProtect, true);
	delay(1);
	return result;
}

void I2CEeprom::SetSlaveAddressFromTargetMemoryAddress( uint16_t targetAddress ) {
	SetAddress( (targetAddress & 0x100) ? (i2cAddress | 0x01) : i2cAddress );
}

#endif // SUPPORT_I2C_SENSORS && SUPPORT_EEPROM_MEMORY

// End
