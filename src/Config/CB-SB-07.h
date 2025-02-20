/*
 * CB-SB-06.h
 *
 *  Created on: 30 May 2022
 *      Author: BigRep
 *
 *      This is the second version of the stage with a microcontroller.
 */

#ifndef SRC_CONFIG_CB_SB_V06_H_
#define SRC_CONFIG_CB_SB_V06_H_

#include <Hardware/PinDescription.h>

#define BOARD_TYPE_NAME		"CB-SB-07"
#define BOOTLOADER_NAME		"CB-SB-07"

// #define SINGLE_MOTOR_STAGE

#define HAS_VREF_MONITOR		0
#define HAS_VOLTAGE_MONITOR		1
#define HAS_12V_MONITOR			0
#define HAS_CPU_TEMP_SENSOR		1
#define HAS_ADDRESS_SWITCHES	0
#define HAS_BUTTONS				0

// Drivers configuration
#define SUPPORT_DRIVERS			1
#define HAS_SMART_DRIVERS		1
#define HAS_STALL_DETECT		1
#ifdef SINGLE_MOTOR_STAGE
#define SINGLE_DRIVER			1
#else
#define SINGLE_DRIVER			0
#endif
#define SUPPORT_SLOW_DRIVERS	0
#define SUPPORT_DELTA_MOVEMENT	0
#define DEDICATED_STEP_TIMER	1
#define SUPPORT_BRAKE_PWM		1
#define USE_SERIAL_DEBUG		0
#define SUPPORT_INPUT_SHAPING	0

#define ACTIVE_HIGH_STEP		1		// 1 = active high, 0 = active low
#define ACTIVE_HIGH_DIR			1		// 1 = active high, 0 = active low

#define SUPPORT_TMC51xx			0
#define SUPPORT_TMC2160			0
#define SUPPORT_TMC2660			0
#define SUPPORT_TMC22xx			1
#define SUPPORT_TMC_RESULT		1

#define SUPPORT_THERMISTORS		1
#define SUPPORT_SPI_SENSORS		0
#define SUPPORT_I2C_SENSORS		1
#define SUPPORT_LIS3DH			0
#define SUPPORT_ADXL345			1
#define SUPPORT_SSD1306			0
#define SUPPORT_LP50XX			0
#define SUPPORT_PCA9685			0
#define SUPPORT_MLX90614		0
#define SUPPORT_NAU7802			0
#define SUPPORT_SHT31			1
#define SUPPORT_LED_STRIPS		0
#define SUPPORT_ADCSTREAM		1

#define SUPPORT_FILAMENT_SENSOR	0

#define SUPPORT_DHT_SENSOR		0

#define USE_MPU					0
#define USE_CACHE				1


constexpr bool UseAlternateCanPins = false;

// Board type & CAN Address ---------------------------------------------
constexpr size_t NumAddressBits = 0;
constexpr size_t NumBoardTypeBits = 1;
constexpr Pin BoardTypePin = { PortBPin(6) };
constexpr Pin BoardAddressPins = NoPin;

// Thermistors & Heaters ------------------------------------------------
constexpr size_t NumThermistorInputs = 5;
constexpr float DefaultThermistorSeriesR = 220.0;
constexpr float VrefTopResistor = 15;
constexpr float MinVrefLoadR = (DefaultThermistorSeriesR / NumThermistorInputs) * 4700.0/((DefaultThermistorSeriesR / NumThermistorInputs) + 4700.0);

constexpr size_t MaxPortsPerHeater = 1;

constexpr Pin TempSensePins[NumThermistorInputs] 		= { PortBPin(9),  PortBPin(8), PortAPin(5), PortAPin(6), PortAPin(7) };

// Motor Drivers --------------------------------------------------------
#ifdef SINGLE_MOTOR_STAGE
constexpr size_t NumDrivers = 1;
constexpr size_t MaxSmartDrivers = 1;

#define TMC22xx_USES_SERCOM				1
#define TMC22xx_HAS_MUX					0
#define TMC22xx_SINGLE_DRIVER			1
#define TMC22xx_HAS_ENABLE_PINS			0
#define TMC22xx_VARIABLE_NUM_DRIVERS	0
#define TMC22xx_USE_SLAVEADDR			0
#define TMC22xx_DEFAULT_STEALTHCHOP		0
#else
constexpr size_t NumDrivers = 2;
constexpr size_t MaxSmartDrivers = 2;

#define TMC22xx_USES_SERCOM				1
#define TMC22xx_HAS_MUX					0
#define TMC22xx_SINGLE_DRIVER			0
#define TMC22xx_HAS_ENABLE_PINS			0
#define TMC22xx_VARIABLE_NUM_DRIVERS	0
#define TMC22xx_USE_SLAVEADDR			1
#define TMC22xx_DEFAULT_STEALTHCHOP		0

#endif

constexpr uint8_t TMC22xxSercomNumber = 5;
Sercom * const SERCOM_TMC22xx = SERCOM5;


constexpr Pin TMC22xxSercomTxPin = PortBPin(16);
constexpr GpioPinFunction TMC22xxSercomTxPinPeriphMode = GpioPinFunction::C;
constexpr Pin TMC22xxSercomRxPin = PortBPin(17);
constexpr GpioPinFunction TMC22xxSercomRxPinPeriphMode = GpioPinFunction::C;
constexpr uint8_t TMC22xxSercomRxPad = 1;			// SERCOM5/PAD[1]


// Define the baud rate used to send/receive data to/from the drivers.
// If we assume a worst case clock frequency of 8MHz then the maximum baud rate is 8MHz/16 = 500kbaud.
// We send data via a 1K series resistor. Even if we assume a 200pF load on the shared UART line, this gives a 200ns time constant, which is much less than the 2us bit time @ 500kbaud.
// To write a register we need to send 8 bytes. To read a register we send 4 bytes and receive 8 bytes after a programmable delay.
// So at 500kbaud it takes about 128us to write a register, and 192us+ to read a register.
// In testing I found that 500kbaud was not reliable on the Duet Maestro, so now using 200kbaud.
constexpr uint32_t DriversBaudRate = 200000;
constexpr uint32_t TransferTimeout = 10;									// any transfer should complete within 10 ticks @ 1ms/tick

constexpr float DriverSenseResistor = 0.091 + 0.02 + 0.003;					// in ohms. Added the 0.003 to make the max current a round 1600mA.
constexpr float DriverVRef = 180.0;											// in mV
constexpr float DriverFullScaleCurrent = DriverVRef/DriverSenseResistor;	// in mA
constexpr float DriverCsMultiplier = 32.0/DriverFullScaleCurrent;
constexpr float MaximumMotorCurrent = 1600.0;
constexpr float MaximumStandstillCurrent = 1200.0;
constexpr uint32_t DefaultStandstillCurrentPercent = 75;

PortGroup * const StepPio = &(PORT->Group[0]);		// the PIO that all the step pins are on
#ifdef SINGLE_MOTOR_STAGE
constexpr Pin StepPins[NumDrivers] 				= { PortAPin(21) };
constexpr Pin DirectionPins[NumDrivers] 		= { PortAPin(20) };
constexpr Pin DriverDiagPins[NumDrivers]		= { PortAPin(22) };
#else
constexpr Pin StepPins[NumDrivers] 				= { PortAPin(21), PortAPin(24) };
constexpr Pin DirectionPins[NumDrivers] 		= { PortAPin(20), PortAPin(23) };
constexpr Pin DriverDiagPins[NumDrivers]		= { PortAPin(22), PortAPin(25) };

#endif

constexpr Pin GlobalTmc22xxEnablePin 			= PortAPin(27);

// Diagnostic LEDs ------------------------------------------------------
constexpr Pin LedPins[] 	= { PortBPin(23), PortBPin(22) };
constexpr bool LedActiveHigh = false;

// Monitor --------------------------------------------------------------
constexpr Pin VinMonitorPin = PortBPin(4);
constexpr float VinDividerRatio = (60.4 + 4.7)/4.7;
constexpr float VinMonitorVoltageRange = VinDividerRatio * 3.3;		// We use the 3.3V supply as the voltage reference

//constexpr Pin VrefPin = PortBPin(4);

//constexpr Pin VssaPin = PortBPin(6);

// I2C Bus --------------------------------------------------------------
#if SUPPORT_I2C_SENSORS

// I2C using pins PA16,17
constexpr uint8_t I2CSercomNumber = 2;
constexpr Pin I2CSDAPin = PortAPin(9);
constexpr GpioPinFunction I2CSDAPinPeriphMode = GpioPinFunction::D;
constexpr Pin I2CSCLPin = PortAPin(8);
constexpr GpioPinFunction I2CSCLPinPeriphMode = GpioPinFunction::D;
#define I2C_HANDLER0		SERCOM2_0_Handler
#define I2C_HANDLER1		SERCOM2_1_Handler
#define I2C_HANDLER3		SERCOM2_2_Handler

#endif

#if SUPPORT_ADXL345
constexpr Pin ADXL345Int1Pin 		= PortBPin(3);
#define SUPPORT_ADXL345_ALT_ADDR
#endif

// Table of pin functions that we are allowed to use
constexpr PinDescription PinTable[] =
{
	//	TC					TCC					ADC					SERCOM in			SERCOM out	  Exint PinName
	// Port A
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PA00 unused
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PA01 Emergency LED
	{ TcOutput::none,	TccOutput::none,	AdcInput::adc0_0,	SercomIo::none,		SercomIo::none,		2,  "emerg"			},	// PA02 EMERGENCY
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx, nullptr			},	// PA03 unused
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PA04 BOARD TYPE
	{ TcOutput::none,	TccOutput::none,	AdcInput::adc0_5,	SercomIo::none,		SercomIo::none,		Nx, "hall0"		    },	// PA05 ADC_HS0
	{ TcOutput::none,	TccOutput::none,	AdcInput::adc0_6,	SercomIo::none,		SercomIo::none,	    Nx, "hall1"		    },	// PA06 ADC_HS1
	{ TcOutput::none,	TccOutput::none,	AdcInput::adc0_7,	SercomIo::none,		SercomIo::none,		Nx, "ball"			},	// PA07 ADC_BS
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx, nullptr			},	// PA08 I2C | SCL
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PA09 I2C | SDA
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr	        },	// PA10 unused
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr		    },	// PA11 unused
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		12,	"io2.in"		},	// PA12 IO_2_IN
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PA13 unused
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PA14 crystal
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PA15 crystal
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PA16 unused
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PA17 unused
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr 		},	// PA18 unused
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PA19 unused
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	"ate.d0.dir"	},	// PA20 MOTOR 0 | Direction
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	"ate.d0.step"	},	// PA21 MOTOR 0 | Step
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		6,	"ate.d0.diag0"  },	// PA22 MOTOR 0 | Diag
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	"ate.d1.dir"	},	// PA23 MOTOR 1 | Direction
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx, "ate.d1.step"	},	// PA24 MOTOR 1 | Step
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		9,	"ate.d1.diag0"	},	// PA25 MOTOR 1 | Diag
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PA26 not on chip
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx, nullptr			},	// PA27 MOTOR ENN
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PA28 not on chip
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PA29 not on chip
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PA30 swclk
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PA31 swdio

	// Port B
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		0,	"io4.in"    	},	// PB00 IO_4_IN
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		1,	"io5.in" 		},	// PB01 IO_5_IN
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx, nullptr 		},	// PB02 unused
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		3,  nullptr 		},	// PB03 ACCEL_INT
	{ TcOutput::none,	TccOutput::none,	AdcInput::adc1_6,	SercomIo::none,		SercomIo::none,		Nx,	"ate.vin"		},	// PB04 ADC_24V
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx, nullptr			},	// PB05 unused
	{ TcOutput::none,	TccOutput::none,	AdcInput::adc1_8,   SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB06 ADC_VSSA
	{ TcOutput::none,	TccOutput::none,	AdcInput::adc1_9,	SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB07 ADC_VREF
	{ TcOutput::none,	TccOutput::none,	AdcInput::adc1_0,	SercomIo::none,		SercomIo::none,		Nx,	"temp1"			},	// PB08	ADC_AN1
	{ TcOutput::none,	TccOutput::none,	AdcInput::adc1_1,	SercomIo::none,		SercomIo::none,		Nx,	"temp0"			},	// PB09 ADC_AN0
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		10, "io0.in"		},	// PB10 IO_0_IN
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		11, "io3.in"		},	// PB11 IO_3_IN
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB12 CAN TX
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB13 CAN RX
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		14, "io1.in"		},	// PB14 IO_1_IN
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr 		},	// PB15 unused
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB16 MOTOR_USART_TX
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB17 MOTOR_USART_RX
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB18 not on chip
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB19 not on chip
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB20 not on chip
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB21 not on chip
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx, nullptr			},	// PB22 LED_ACT
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx, nullptr 		},	// PB23 LED_STATUS Green
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB24 not on chip
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB25 not on chip
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB26 not on chip
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB27 not on chip
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB28 not on chip
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB29 not on chip
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx, nullptr		    },	// PB30 unused
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx, nullptr		    },	// PB31 unused
};

static constexpr size_t NumPins = ARRAY_SIZE(PinTable);
static constexpr size_t NumRealPins = 32 + 32;			// 32 pins on port A, 32 on port B
static_assert(NumPins == NumRealPins);					// no virtual pins

// Timer/counter used to generate step pulses and other sub-millisecond timings
TcCount32 * const StepTc = &(TC0->COUNT32);
constexpr IRQn StepTcIRQn = TC0_IRQn;
constexpr unsigned int StepTcNumber = 0;
#define STEP_TC_HANDLER		TC0_Handler

// Available UART ports
#define NUM_SERIAL_PORTS		0

// DMA channel assignments. Channels 0-3 have individual interrupt vectors, channels 4-31 share an interrupt vector.
constexpr DmaChannel DmacChanTmcTx  = 0;
constexpr DmaChannel DmacChanTmcRx  = 1;

constexpr unsigned int NumDmaChannelsUsed = 2;			// must be at least the number of channels used, may be larger. Max 32 on the SAME51.
constexpr DmaPriority DmacPrioTmcTx = 0;
constexpr DmaPriority DmacPrioTmcRx = 1;

// Interrupt priorities, lower means higher priority. 0-2 can't make RTOS calls.
const NvicPriority NvicPriorityStep = 3;				// step interrupt is next highest, it can preempt most other interrupts
const NvicPriority NvicPriorityUart = 3;				// serial driver makes RTOS calls
const NvicPriority NvicPriorityPins = 3;				// priority for GPIO pin interrupts
const NvicPriority NvicPriorityCan  = 4;
const NvicPriority NvicPriorityDmac = 5;				// priority for DMA complete interrupts
const NvicPriority NvicPriorityAdc  = 5;
const NvicPriority NvicPriorityI2C  = 6;

#endif /* SRC_CONFIG_CB_SB_V06_H_ */
