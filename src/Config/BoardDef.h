/*
 * BoardDef.h
 *
 *  Created on: 30 Jun 2019
 *      Author: David
 */

#ifndef SRC_CONFIG_BOARDDEF_H_
#define SRC_CONFIG_BOARDDEF_H_

#include <Duet3Common.h>								// this file is in the CANlib project because both main and expansion boards need it
#include <RRF3Common.h>

#if defined(EXP3HC)
# include "EXP3HC.h"
#elif defined(TOOL1LC)
# include "TOOL1LC.h"
#elif defined(TOOL1RR)
# include "TOOL1RR.h"
#elif defined(EXP1XD)
# include "EXP1XD.h"
#elif defined(EXP1HCL)
# include "EXP1HCL.h"
#elif defined(SAMMYC21)
# include "SAMMYC21.h"
#elif defined(ATECM)
# include "ATECM.h"
#elif defined(ATEIO)
# include "ATEIO.h"
#elif defined(RPI_PICO)
# include "RPi_Pico.h"
#elif defined(M23CL)
# include "M23CL.h"
#elif defined(CB_MP_02)
# include "CB-MP-02.h"
#elif defined(CB_MP_03)
# include "CB-MP-03.h"
#elif defined(CB_MP_04)
# include "CB-MP-04.h"
#elif defined(CB_MP_05)
# include "CB-MP-05.h"
#elif defined(CB_MT_01)
# include "CB-MT-01.h"
#elif defined(CB_SE_02)
# include "CB-SE-02.h"
#elif defined(CB_SE_03)
# include "CB-SE-03.h"
#elif defined(CB_SE_04)
# include "CB-SE-04.h"
#elif defined(CB_CX_01)
# include "CB-CX-01.h"
#elif defined(CB_CX_02)
# include "CB-CX-02.h"
#elif defined(CB_SB_04)
# include "CB-SB-04.h"
#elif defined(CB_SB_06)
# include "CB-SB-06.h"
#elif defined(CB_SB_07)
# include "CB-SB-07.h"
#elif defined(SZP)
# include "SZP.h"
#elif defined(F3PTB)
# include "F3PTB.h"
#else
# error Board type not defined
#endif

// Default board features
#ifndef DIFFERENTIAL_STEPPER_OUTPUTS
# define DIFFERENTIAL_STEPPER_OUTPUTS	0
#endif

#ifndef USE_TC_FOR_STEP
# define USE_TC_FOR_STEP				0
#endif

#ifndef SUPPORT_CLOSED_LOOP
# define SUPPORT_CLOSED_LOOP			0
#endif

#ifndef SUPPORT_PHASE_STEPPING
# define SUPPORT_PHASE_STEPPING			0
#endif

#ifndef SUPPORT_BRAKE_PWM
# define SUPPORT_BRAKE_PWM				0
#endif

#ifndef DEDICATED_STEP_TIMER
# define DEDICATED_STEP_TIMER			0
#endif

#ifndef SUPPORT_OVERRIDE_STEP_PIN
# define SUPPORT_OVERRIDE_STEP_PIN		0
#endif

#if !SUPPORT_DRIVERS
# define HAS_SMART_DRIVERS				0
# define SUPPORT_TMC22xx				0
# define SUPPORT_TMC2660				0
# define SUPPORT_TMC51xx				0
# define SUPPORT_SLOW_DRIVERS			0
constexpr size_t NumDrivers = 0;
#endif

#ifndef SUPPORT_ACT_LED_ON_CAN_MESSAGE
# define SUPPORT_ACT_LED_ON_CAN_MESSAGE	0
#endif

#if !defined(SUPPORT_BME280)
# define SUPPORT_BME280					(SUPPORT_SPI_SENSORS)
#endif

#if !defined(SUPPORT_LIS3DH)
# define SUPPORT_LIS3DH					0
#endif

#if !defined(USE_SERIAL_DEBUG)
# define USE_SERIAL_DEBUG				0
#endif

#ifndef SUPPORT_LED_STRIPS
# define SUPPORT_LED_STRIPS				1
#endif

#ifndef SUPPORT_ANALOG_THRESHOLD
# define SUPPORT_ANALOG_THRESHOLD 		1
#endif

#ifndef SUPPORT_BOOTLOADER_OM
# define SUPPORT_BOOTLOADER_OM			1
#endif

#ifndef SUPPORT_DMA_NEOPIXEL
# define SUPPORT_DMA_NEOPIXEL			0
#endif

#ifndef SUPPORT_PIO_NEOPIXEL
# define SUPPORT_PIO_NEOPIXEL			(SUPPORT_LED_STRIPS && RP2040)
#endif

#ifndef SUPPORT_FILAMENT_SENSOR
# define SUPPORT_FILAMENT_SENSOR			0
#endif

#ifndef SUPPORT_ADXL345
# define SUPPORT_ADXL345				0
#endif

#ifndef SUPPORT_LED_STATES
# define SUPPORT_LED_STATES				0
#endif

#ifndef SUPPORT_PLASMICS_INO
# define SUPPORT_PLASMICS_INO			0
#endif

#ifndef SUPPORT_LP50XX
# define SUPPORT_LP50XX					0
#endif

#ifndef SUPPORT_TMC_RESULT
# define SUPPORT_TMC_RESULT				0
#endif

#ifndef SUPPORT_PCA9685
# define SUPPORT_PCA9685				0
#endif

#ifndef SUPPORT_NAU7802
# define SUPPORT_NAU7802 				0
#endif

#ifndef SUPPORT_MLX90614
# define SUPPORT_MLX90614				0
#endif

#ifndef SUPPORT_SHT31
# define SUPPORT_SHT31 					0
#endif

#ifndef SUPPORT_EEPROM_MEMORY
# define SUPPORT_EEPROM_MEMORY			0
#endif

#ifndef SUPPORT_TMC5160_LUT
# define SUPPORT_TMC5160_LUT				0
#endif

#ifndef SUPPORT_ADCSTREAM
# define SUPPORT_ADCSTREAM				0
#endif

#ifndef SUPPORT_PT100
# define SUPPORT_PT100					0
#endif

#ifndef HAS_48V_MONITOR
# define HAS_48V_MONITOR 				0
#endif

#ifndef SUPPORT_LDC1612
# define SUPPORT_LDC1612				0
#endif

#ifndef SUPPORT_AS5601
# define SUPPORT_AS5601					0
#endif

#ifndef SUPPORT_TCA6408A
# define SUPPORT_TCA6408A				0
#endif

#ifndef USE_SPICAN
# define USE_SPICAN						0
#endif

#ifndef BOARD_USES_UF2_BINARY
# define BOARD_USES_UF2_BINARY			0
#endif

#ifndef ACCELEROMETER_USES_SPI
# define ACCELEROMETER_USES_SPI         0
#endif

#endif /* SRC_CONFIG_BOARDDEF_H_ */
