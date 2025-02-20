/*
 * CB-MP-04.h
 *
 *  Created on: 10 May 2022
 *      Author: BigRep
 */

#ifndef SRC_CONFIG_CB_MP_V04_H_
#define SRC_CONFIG_CB_MP_V04_H_

#include <Hardware/PinDescription.h>

#define BOARD_TYPE_NAME		"CB-MP-04"
#define BOOTLOADER_NAME		"CB-MP-04"

#define HAS_VREF_MONITOR		1
#define HAS_VOLTAGE_MONITOR		1
#define HAS_12V_MONITOR			0
#define HAS_48V_MONITOR			1
#define HAS_CPU_TEMP_SENSOR		1
#define HAS_ADDRESS_SWITCHES	1
#define HAS_BUTTONS				0

// Drivers configuration
#define SUPPORT_DRIVERS			1
#define HAS_SMART_DRIVERS		1
#define HAS_STALL_DETECT		1
#define SINGLE_DRIVER			0
#define SUPPORT_SLOW_DRIVERS	0
#define SUPPORT_DELTA_MOVEMENT	0
#define DEDICATED_STEP_TIMER	1
#define SUPPORT_BRAKE_PWM		1
#define USE_SERIAL_DEBUG		0
#define SUPPORT_INPUT_SHAPING	1

#define ACTIVE_HIGH_STEP		1		// 1 = active high, 0 = active low
#define ACTIVE_HIGH_DIR			1		// 1 = active high, 0 = active low

#define SUPPORT_TMC51xx			1
#define SUPPORT_TMC2160			0
#define SUPPORT_TMC22xx			0
#define SUPPORT_TMC2660			0

#define SUPPORT_THERMISTORS		1
#define SUPPORT_SPI_SENSORS		0
#define SUPPORT_I2C_SENSORS		1
#define SUPPORT_LIS3DH			0
#define SUPPORT_ADXL345			1
#define SUPPORT_SSD1306			0
#define SUPPORT_LP50XX			0
#define SUPPORT_PCA9685			0
#define SUPPORT_MLX90640		0
#define SUPPORT_SHT31			1
#define SUPPORT_LED_STRIPS		0
#define SUPPORT_ADCSTREAM		0

#define SUPPORT_TMC_RESULT		1
#define SUPPORT_TMC5160_LUT		0

#define SUPPORT_DHT_SENSOR		0

#define USE_MPU					0
#define USE_CACHE				1


// Thermistors & Heaters ------------------------------------------------
constexpr size_t NumThermistorInputs = 3;
constexpr float DefaultThermistorSeriesR = 2200.0;
constexpr float VrefTopResistor = 15.0;
constexpr float MinVrefLoadR = (DefaultThermistorSeriesR / NumThermistorInputs) * 4700.0/((DefaultThermistorSeriesR / NumThermistorInputs) + 4700.0);
constexpr size_t MaxPortsPerHeater = 0;

constexpr Pin TempSensePins[NumThermistorInputs] 		= { PortBPin(7), PortBPin(9), PortBPin(8) };

// Motor Drivers --------------------------------------------------------
constexpr size_t NumDrivers 							= 2;
constexpr size_t MaxSmartDrivers 						= 2;
constexpr float MaxTmc5160Current 						= 6300.0;			// The maximum current we allow the TMC5160/5161 drivers to be set to
constexpr uint32_t DefaultStandstillCurrentPercent 		= 71;
constexpr float Tmc5160SenseResistor 					= 0.050;

constexpr Pin GlobalTmc51xxEnablePin 					= PortAPin(27);
constexpr Pin GlobalTmc51xxCSPin 						= PortAPin(18);

#define TMC51xx_USES_SERCOM	1
Sercom * const SERCOM_TMC51xx 							= SERCOM1;
constexpr uint8_t SERCOM_TMC51xx_NUMBER 				= 1;

constexpr Pin TMC51xxMosiPin 							= PortAPin(16);
constexpr GpioPinFunction TMC51xxMosiPinPeriphMode 		= GpioPinFunction::C;
constexpr Pin TMC51xxSclkPin 							= PortAPin(17);
constexpr GpioPinFunction TMC51xxSclkPinPeriphMode 		= GpioPinFunction::C;
constexpr Pin TMC51xxMisoPin 							= PortAPin(19);
constexpr GpioPinFunction TMC51xxMisoPinPeriphMode 		= GpioPinFunction::C;

PortGroup * const StepPio 								= &(PORT->Group[0]);		// the PIO that all the step pins are on
constexpr Pin StepPins[NumDrivers] 						= { PortAPin(21), PortAPin(24) };
constexpr Pin DirectionPins[NumDrivers] 				= { PortAPin(20), PortAPin(23) };

// Board type & CAN Address ---------------------------------------------
constexpr size_t NumAddressBits 						= 3;
constexpr size_t NumBoardTypeBits						= 0;
constexpr bool UseAlternateCanPins 						= false;

constexpr Pin BoardTypePins[NumBoardTypeBits] 			= { };
constexpr Pin BoardAddressPins[NumAddressBits] 			= { PortBPin(16), PortAPin(0), PortAPin(1) };

// Diagnostic LEDs ------------------------------------------------------
constexpr Pin LedPins[] 								= { PortBPin(23), PortBPin(22) };  // { Status, Act }
constexpr bool LedActiveHigh 							= false;

// Monitor --------------------------------------------------------------
constexpr Pin   VinMonitorPin 							= PortBPin(4);
constexpr float VinDividerRatio 						= (60.4 + 4.7)/4.7;
constexpr float VinMonitorVoltageRange 					= VinDividerRatio * 3.3;		// We use the 3.3V supply as the voltage reference

constexpr Pin   V48MonitorPin 							= PortAPin(3);
constexpr float V48DividerRatio 						= (120.0 + 4.7)/4.7;
constexpr float V48MonitorVoltageRange 					= V48DividerRatio * 3.3;		// We use the 3.3V supply as the voltage reference

constexpr Pin VrefPin = PortBPin(5);
constexpr Pin VssaPin = PortBPin(6);


#if defined(SUPPORT_CLOSED_LOOP)
#if SUPPORT_CLOSED_LOOP
#if SUPPORT_I2C_SENSORS
#error The SerCom0 is shared between the Encoder and the I2C.
#endif
// Encoder --------------------------------------------------------------
// Encoder and quadrature decoder interface
constexpr Pin QuadratureResetPin 						= PortBPin(2); // The reset is the USART_TX
constexpr Pin EncoderCsPin 								= PortAPin(6); // It is also the board type

// Shared SPI (used for interface to encoders, not for temperature sensors)
constexpr uint8_t EncoderSspiSercomNumber 				= 0;	// Same that I2C. We can't share.
constexpr uint32_t EncoderSspiDataInPad 				= 3;
constexpr Pin EncoderMosiPin 							= PortAPin(4);
constexpr Pin QuadratureErrorOutPin 					= EncoderMosiPin;
constexpr GpioPinFunction EncoderMosiPinPeriphMode 		= GpioPinFunction::C;

constexpr Pin EncoderSclkPin 							= PortAPin(5);
constexpr Pin QuadratureCountUpPin 						= EncoderSclkPin;
constexpr GpioPinFunction EncoderSclkPinPeriphMode 		= GpioPinFunction::C;

constexpr Pin EncoderMisoPin 							= PortAPin(7);
constexpr Pin QuadratureCountDownPin 					= EncoderMisoPin;
constexpr GpioPinFunction EncoderMisoPinPeriphMode 		= GpioPinFunction::C;

// Clock generator pin for external devices
constexpr uint8_t ClockGenGclkNumber 					= 4;
constexpr Pin ClockGenPin 								= PortAPin(10);
constexpr GpioPinFunction ClockGenPinPeriphMode 		= GpioPinFunction::M;
#endif
#endif

// I2C -----------------------------------------------------------------
#if SUPPORT_I2C_SENSORS

constexpr uint8_t I2CSercomNumber = 2;
constexpr Pin I2CSDAPin = PortAPin(9);
constexpr GpioPinFunction I2CSDAPinPeriphMode = GpioPinFunction::D;
constexpr Pin I2CSCLPin = PortAPin(8);
constexpr GpioPinFunction I2CSCLPinPeriphMode = GpioPinFunction::D;
#define I2C_HANDLER0		SERCOM2_0_Handler
#define I2C_HANDLER1		SERCOM2_1_Handler
#define I2C_HANDLER3		SERCOM2_2_Handler

#if SUPPORT_ADXL345
constexpr Pin ADXL345Int1Pin 		= PortBPin(3);
#define SUPPORT_ADXL345_ALT_ADDR
#endif

#endif

// Table of pin functions that we are allowed to use
constexpr PinDescription PinTable[] =
{
	//	TC					TCC					ADC					SERCOM in			SERCOM out	  Exint PinName
	// Port A
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PA00 Address | 0
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PA01 Address | 1
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		2,	"emerg"			},	// PA02 Emergency In
	{ TcOutput::none,	TccOutput::none,	AdcInput::adc0_1,	SercomIo::none,		SercomIo::none,		Nx, "ate.v48"		},	// PA03 ADC 	| Vin 48V
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	"tfr"			},	// PA04 Encoder | MOSI
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx, nullptr			},	// PA05 Encoder | CLK
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx, "tfy"			},	// PA06 BoardType & Encoder | CS
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx, "tfg"			},	// PA07 Encoder | MISO
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx, nullptr			},	// PA08 I2C 	| SCL
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx, nullptr			},	// PA09 I2C 	| SDA
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PA10 Encoder | PWM
	{ TcOutput::none,	TccOutput::tcc1_7G,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	"out0"			},	// PA11 Output  | Out 0
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		12,	"io2.in"		},	// PA12 Input 	| 24V Input 1
	{ TcOutput::tc2_1,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	"out3"			},	// PA13 Output  | Fan 3
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PA14 crystal
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PA15 crystal
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PA16 Motors	| SPI MOSI
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PA17 Motors	| SPI CLK
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr 		},	// PA18 Motors	| SPI CS
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PA19 Motors	| SPI MISO
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	"ate.d0.dir"	},	// PA20 Motor 0	| Direction
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	"ate.d0.step"	},	// PA21 Motor 0	| Step
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		6,	"ate.d0.diag0"	},	// PA22 Motor 0	| Diagnostic 0
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	"ate.d1.dir"	},	// PA23 Motor 1 | Direction
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx, "ate.d1.step"	},	// PA24 Motor 1	| Step
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		9,	"ate.d1.diag0"	},	// PA25 Motor 1	| Diagnostic 0
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PA26 not on chip
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx, nullptr			},	// PA27 Motors	| Enable
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PA28 not on chip
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PA29 not on chip
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PA30 SWD 	| SWCLK
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PA31 SWD		| SWDIO

	// Port B
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		0,	"io4.in"		},	// PB00 Input 	| 3.3V Input 5
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		1,	"io5.in"		},	// PB01 Input 	| 3.3V Input 4
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx, nullptr			},	// PB02 Encoder | USART Tx or ENC Reset
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		3, nullptr			},	// PB03 Accelerometer |  INT
	{ TcOutput::none,	TccOutput::none,	AdcInput::adc1_6,	SercomIo::none,		SercomIo::none,		Nx,	"ate.vin"		},	// PB04 ADC 	| Vin 24V
	{ TcOutput::none,	TccOutput::none,	AdcInput::adc1_7,	SercomIo::none,		SercomIo::none,		Nx, nullptr			},	// PB05 ADC 	| VrefMon
	{ TcOutput::none,	TccOutput::none,	AdcInput::adc1_8,	SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB06 ADC 	| VssaMon
	{ TcOutput::none,	TccOutput::none,	AdcInput::adc1_9,	SercomIo::none,		SercomIo::none,		Nx,	"temp1"			},	// PB07 ADC 	| Temperature 1
	{ TcOutput::none,	TccOutput::none,	AdcInput::adc1_0,	SercomIo::none,		SercomIo::none,		8,	"temp2,lin0"	},	// PB08 ADC 	| Temperature 2 or Linear 0
	{ TcOutput::none,	TccOutput::none,	AdcInput::adc1_1,	SercomIo::none,		SercomIo::none,		Nx,	"temp0"			},	// PB09 ADC 	| Temperature 0
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		10, "io0.in"		},	// PB10 Input 	| 24V Input 2
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		11, "io3.in"		},	// PB11 Input 	| 24V Input 3
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB12 CAN		| Tx
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB13 CAN		| Rx
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		14, "io1.in"		},	// PB14 Input 	| 24V Input 0
	{ TcOutput::tc5_1,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	"out1"			},	// PB15 Output  | Output 1
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB16 Address | 2
	{ TcOutput::none,	TccOutput::tcc3_1F,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	"out2"			},	// PB17 Output  | Output 2
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB18 not on chip
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB19 not on chip
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB20 not on chip
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB21 not on chip
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx, nullptr			},	// PB22 LED		| Activity
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx, nullptr			},	// PB23 LED		| Status
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB24 not on chip
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB25 not on chip
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB26 not on chip
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB27 not on chip
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB28 not on chip
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	nullptr			},	// PB29 not on chip
	{ TcOutput::none,	TccOutput::tcc4_0F,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	"out4"			},	// PB30 Output  | 3.3V Output 4
	{ TcOutput::none,	TccOutput::none,	AdcInput::none,		SercomIo::none,		SercomIo::none,		Nx,	"out5"			},	// PB31 Output  | 3.3V Output 5
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
#define NUM_SERIAL_PORTS		1
constexpr IRQn Serial0_IRQn = SERCOM5_0_IRQn;

// DMA channel assignments. Channels 0-3 have individual interrupt vectors, channels 4-31 share an interrupt vector.
constexpr DmaChannel DmacChanTmcTx  = 0;
constexpr DmaChannel DmacChanTmcRx  = 1;

constexpr unsigned int NumDmaChannelsUsed = 2;			// must be at least the number of channels used, may be larger. Max 32 on the SAME51.

constexpr DmaPriority DmacPrioTmcTx = 0;
constexpr DmaPriority DmacPrioTmcRx = 3;

// Interrupt priorities, lower means higher priority. 0-2 can't make RTOS calls.
const NvicPriority NvicPriorityStep = 3;				// step interrupt is next highest, it can preempt most other interrupts
const NvicPriority NvicPriorityUart = 3;				// serial driver makes RTOS calls
const NvicPriority NvicPriorityPins = 3;				// priority for GPIO pin interrupts
const NvicPriority NvicPriorityCan  = 4;
const NvicPriority NvicPriorityDmac = 5;				// priority for DMA complete interrupts
const NvicPriority NvicPriorityAdc  = 5;
const NvicPriority NvicPriorityI2C  = 6;

#endif /* SRC_CONFIG_CB_MP_V04_H_ */
