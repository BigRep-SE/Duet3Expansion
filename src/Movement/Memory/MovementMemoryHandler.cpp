/*
 * MovementMemoryHandler.cpp
 *
 *  Created on: 20.10.2023
 *      Author: BigRep
 */

#include <Movement/Memory/MovementMemoryHandler.h>

#if SUPPORT_I2C_SENSORS && SUPPORT_EEPROM_MEMORY

#include <RTOSIface/RTOSIface.h>
#include <Platform/TaskPriorities.h>

#include <Movement/Memory/I2CEeprom.h>
#include "Movement/DDA.h"


constexpr size_t FilamentMemoryTaskStackWords = 350;
static Task<FilamentMemoryTaskStackWords> *filamentMemoryTask {nullptr};

static I2CEeprom			*memory { nullptr };

uint8_t GetParity(const uint8_t *p, const size_t length){
	// Calculate the parity.
	uint8_t parity = 0;
	for( size_t i = 0; i < length ; ++i ) {
		parity ^= p[i];
	}
	return parity;
}


struct __attribute__((packed)) ExtrusionData {

	uint32_t time 		{0xFFFFFFFF};	///< Time when this was created.
	uint32_t forward	{0xFFFFFFFF};	///< Forwared moves (extrusion)
	uint32_t backward	{0xFFFFFFFF};	///< Retractions
	uint8_t  parity		{0x00};			///< Basic CRC to check that the data is valid

	void Set( const uint32_t _time, const uint32_t _forward, const uint32_t _backward ) {
		time = _time;
		forward = _forward;
		backward = _backward;

		// Calculate the parity.
		parity = GetParity((const uint8_t*)this, (sizeof(ExtrusionData)-1));
	}
	bool Check( ) const {
		return (GetParity((const uint8_t*)this, (sizeof(ExtrusionData))) == 0);
	}
};

static_assert( sizeof(ExtrusionData) == ((sizeof(uint32_t)*3) + 1) );
inline bool operator==(const ExtrusionData& lhs, const ExtrusionData& rhs) {
	return lhs.time == rhs.time && lhs.forward == rhs.forward && lhs.backward == rhs.backward && lhs.parity == rhs.parity;
}
enum ExtrusionScale : uint8_t{
	Unknown = 0,		///< Not valid
	N1m 	= 0xFC,		///< The values stored must be multiplied by 1m
	N5m		= 0xFD,		///< The values stored must be multiplied by 5m
	N10m 	= 0xFE		///< The values stored must be multiplied by 10m
};

struct __attribute__((packed)) MemoryInformation{
	uint32_t timeFirstConnection {0xFFFFFFFF};		///< First time that the extruder was connected.
	uint8_t version {0xFF};							///< Version used in this memory->
	ExtrusionScale scale{ExtrusionScale::Unknown};	///< Stores the scale used in the memory: 1 = 1mm , 2 = 5mm, 3 = 10mm
	uint8_t parity {0};								///< Basic parity to check that the data is valid

	void Set( const uint32_t _time, const uint8_t _version, const ExtrusionScale _scale ) {
		timeFirstConnection = _time;
		version = _version;
		scale = _scale;
		// Calculate the parity.
		parity = GetParity((const uint8_t*)this, (sizeof(MemoryInformation)-1));
	}
	bool Check( ) const {
		if((GetParity((const uint8_t*)this, (sizeof(MemoryInformation))) == 0)){
			return (version == 1) && ((N1m <= scale) && (scale <= N10m));
		}
		return false;
	}
};
static_assert( sizeof(MemoryInformation) == (sizeof(uint32_t) + 3) );

class MemoryController {
public:
	inline static constexpr size_t INFORMATION_ADDRESS 			= 0;
	inline static constexpr size_t DATA_EXTRUSION_BLOCKS 		= (I2CEeprom::MEMORY_SIZE-(sizeof(MemoryInformation)))/sizeof(ExtrusionData) ;
	inline static constexpr size_t DATA_EXTRUSION_START_ADDRESS = I2CEeprom::MEMORY_SIZE-(DATA_EXTRUSION_BLOCKS*sizeof(ExtrusionData));

	MemoryController(  ) { };

	bool Init() {
		// Reset();
		if( !memory->IsInit() ){
			memory->Init();
		}
		// If still not init, there is a problem
		if( !memory->IsInit() ) {
			return false;
		}
		if( !LoadInformation( ) ) {
			return DefaultInitialization( );
		}
		if( !IsInit() ){
			return false;
		}
		// Let's read each block
		ExtrusionData loadedData;
		for( size_t i = 0; i < DATA_EXTRUSION_BLOCKS; ++i ) {
			ReadExtrusionDataInIndex( i, loadedData );
			if( loadedData.Check() ){
				if(data.Check()){
					if( data.time < loadedData.time ) {
						// This data is newer
						dataIndex = i;
						data = loadedData;
					}
				}
				else {
					// It is new data
					dataIndex = i;
					data = loadedData;
				}
			}
		}
		return HasValidData() ;
	};

	bool IsInit() const { return information.Check() && memory->IsInit(); };

	const MemoryInformation & Information() const { return information; } ;

	const ExtrusionData & Data() const { return data; } ;

	bool HasValidData( ) const {
		return ( dataIndex < DATA_EXTRUSION_BLOCKS ) && data.Check();
	};

	bool UpdateExtrusionData( const uint32_t time, const uint32_t newForwardDistance, const uint32_t newBackwardDistance ) {
		if( time == 0 ) {
			return false;
		}
		ExtrusionData 		dataToWrite;
		dataToWrite.Set( time , newForwardDistance, newBackwardDistance );
		auto retries = 3; // Max amout of retries in different blocks
		auto newIndex = dataIndex ;
		while(retries > 0){
			newIndex = (newIndex + 1) % DATA_EXTRUSION_BLOCKS;
			--retries;
			if( WriteDataInIndex( newIndex, dataToWrite) ) {
				dataIndex = newIndex;
				data = dataToWrite;
				return true;
			}
		}
		return false;
	}

	void ResetData() {
		dataIndex =  0xFFFF  ;
		data.Set(0xFF, 0, 0);
	}

private:
	inline static constexpr uint8_t VERSION = 1 ;

	MemoryInformation 	information  ;
	size_t 				dataIndex { 0xFFFF } ;
	ExtrusionData 		data { 0xFF, 0, 0 };

	bool WriteDataInIndex( const size_t idx,  const ExtrusionData &dataToWrite ) {
#ifdef DEBUG
		debugPrintf("W|I=%u,T=%u,F=%u,R=%u,P=%u,V=%u\n",
				(unsigned int)idx,
				(unsigned int)dataToWrite.time,
				(unsigned int)dataToWrite.forward,
				(unsigned int)dataToWrite.backward,
				(unsigned int)dataToWrite.parity,
				(unsigned int)dataToWrite.Check());
#endif
		if( idx < DATA_EXTRUSION_BLOCKS && IsInit() && idx != dataIndex ) {
			auto result = memory->Write( DATA_EXTRUSION_START_ADDRESS + idx * sizeof(ExtrusionData),
					(const uint8_t*)&dataToWrite, sizeof(ExtrusionData) );
			if( result ) {
				ExtrusionData dataToCompare;
				result = ReadExtrusionDataInIndex( idx , dataToCompare );
				return ( result && (dataToCompare == dataToWrite) );
			}
		}
		return false;
	}

	bool ReadExtrusionDataInIndex( const size_t idx , ExtrusionData & resultData ){

		if( idx < DATA_EXTRUSION_BLOCKS && IsInit() ){
			auto result = memory->Read( DATA_EXTRUSION_START_ADDRESS + idx * sizeof(ExtrusionData),
				(uint8_t *)&resultData, sizeof(ExtrusionData) );
#ifdef DEBUG
			debugPrintf("R|I=%u,T=%u,F=%u,R=%u,P=%u,V=%u\n",
					(unsigned int)idx,
					(unsigned int)resultData.time,
					(unsigned int)resultData.forward,
					(unsigned int)resultData.backward,
					(unsigned int)resultData.parity,
					(unsigned int)resultData.Check());
#endif
			return result && resultData.Check();
		}
		return false;
	}

	bool LoadInformation( ) {
		if( memory->IsInit() ) {
			MemoryInformation info;
			memory->Read( INFORMATION_ADDRESS , (uint8_t *) &info, sizeof( MemoryInformation ) );
			if( info.Check() ) {
				information = info;
				return true;
			}
		}
		return false;
	};

	bool DefaultInitialization(  ) {
		const uint32_t timeFirstConnection = (uint32_t)Platform::GetDateTime();
		if(timeFirstConnection == 0) {
			return false;
		}
		if( !memory->IsInit() ) {
			return false;
		}

		MemoryInformation newInformation;
		newInformation.Set( timeFirstConnection, VERSION , ExtrusionScale::N1m );
		if( !memory->Write( INFORMATION_ADDRESS, (uint8_t *) &newInformation, sizeof(MemoryInformation) ) ) {
			return false;
		}
		if( !LoadInformation( ) ){
			return false;
		}
		// Create the first and empty extrusion data
		ExtrusionData dataToCompare;
		dataToCompare.Set( timeFirstConnection, 0, 0 );
		if( !WriteDataInIndex( 0, dataToCompare )) {
			return false;
		}
		dataIndex = 0;
		data = dataToCompare;
		return true;
	};


	void Reset(  ) {
		for(uint16_t i = 0; i < I2CEeprom::MEMORY_SIZE ; ++i ){
			uint8_t val = 0;
			memory->Write(i, &val, 1);
		}
	};
};

static MemoryController 	*controller {nullptr};

[[noreturn]] void FilamentMemoryTaskCode(void*) noexcept
{
	constexpr uint32_t minMmToRegister = 1000;
	delay(50);
	controller->Init();

	while( !controller->IsInit() ){
		delay(100); // We can wait for a while
		controller->Init();
	}

	uint32_t lastStepsDone = DDA::extrusionStepsDone[0];
	uint32_t lastStepsRetraction = DDA::retractionStepsDone[0];
	uint32_t lastStepsForward =  lastStepsDone - lastStepsRetraction;
	constexpr auto ChangedEnoughSteps = [] ( uint32_t newValue, uint32_t lastValue, uint32_t stepsDivisor) -> bool {
		return (newValue - lastValue) >= stepsDivisor;
	};

	for (;;)
	{
		const float stepsPerMm = Platform::DriveStepsPerUnit(0);
		const uint32_t stepsDivisor = (uint32_t)(stepsPerMm * (float)minMmToRegister);

		const uint32_t newStepsDone = DDA::extrusionStepsDone[0];
		const uint32_t newStepsRetraction = DDA::retractionStepsDone[0];
		const uint32_t newStepsForward = newStepsDone - newStepsRetraction;
		if( ChangedEnoughSteps( newStepsForward, lastStepsForward, stepsDivisor ) ||
			ChangedEnoughSteps( newStepsRetraction, lastStepsRetraction, stepsDivisor ) )
		{
			const uint32_t diffDistanceForward  =  (newStepsForward - lastStepsForward) 	/ stepsDivisor;
			const uint32_t diffDistanceRetraction =  (newStepsRetraction - lastStepsRetraction) / stepsDivisor;
			const uint32_t distanceForward = controller->Data().forward + diffDistanceForward;
			const uint32_t distanceRetraction = controller->Data().backward + diffDistanceRetraction;
			if(controller->UpdateExtrusionData( Platform::GetDateTime(), distanceForward, distanceRetraction )){
				lastStepsDone 		+= stepsDivisor * (diffDistanceForward + diffDistanceRetraction);
				lastStepsRetraction += stepsDivisor * diffDistanceRetraction;
				lastStepsForward	+= stepsDivisor * diffDistanceForward;

			}
		}
#ifdef DEBUG
		else{
			debugPrintf("T=%u,F=%u,R=%u\n",
					(unsigned int)Platform::GetDateTime(),
					(unsigned int)(lastStepsForward/stepsDivisor),
					(unsigned int)(newStepsRetraction/stepsDivisor));
		}
#endif
		delay(2000); // This can be slow
	}
}

void MovementMemoryHandler::Init( uint32_t _angleRotationRange ) {
	if(memory == nullptr){
		memory = new I2CEeprom(0, I2CEepromPin, true);
	}
	if(controller == nullptr){
		controller = new MemoryController();
	}
	if(filamentMemoryTask == nullptr) {
		filamentMemoryTask = new Task<FilamentMemoryTaskStackWords>;
		filamentMemoryTask->Create(FilamentMemoryTaskCode, "FILAMEM", nullptr, TaskPriority::FilamentMemory);
	}
}

bool MovementMemoryHandler::IsInit() noexcept{
	return controller->IsInit();
}

uint32_t MovementMemoryHandler::GetForwardMeters() noexcept{
	return controller->Data().forward;
}

uint32_t MovementMemoryHandler::GetBackwardMeters() noexcept{
	return controller->Data().backward;
}

#endif // SUPPORT_I2C_SENSORS && SUPPORT_EEPROM_MEMORY
