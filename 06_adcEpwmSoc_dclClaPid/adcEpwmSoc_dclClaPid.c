/**
 * @file        main.c
 * @project		_E_epwm_gpio_toggle
 *
 * @date        May 28, 2017
 * @author      Manuel Del Basso (mainster)
 * @email       manuel.delbasso@gmail.com
 *
 * @ide         Code Composer Studio Version: 7.1.0.00015
 * @license		GNU GPL v3
 *
 @verbatim

 ------------------------------------------------------------------------

 Copyright (C) 2017  Manuel Del Basso

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 ------------------------------------------------------------------------

 @endverbatim
 *
 */
#include <stdint.h>
#include <stdbool.h>
/* ------------------------------------------------------------------------- */
#include "driverlib.h"
#include "device.h"
/* ------------------------------------------------------------------------- */
#include "F28x_Project.h"
#include "F2837xD_GlobalPrototypes.h"
/* ------------------------------------------------------------------------- */
#include "DCL.h"
/* ------------------------------------------------------------------------- */
#include "adcEpwmSoc_dclClaPid.h"
#include "md_globals.h"
#include "md_config.h"
#include "md_helpers.h"
#include "md_epwm.h"
#include "md_sci.h"

#define  Ts	 25e-6

PID pid1 = PID_DEFAULTS;	//!< PID instance

volatile uint16_t adcRes[4] =
	{ 0, 0, 0, 0 };

void ConfigureADC(void);
void ConfigureEPWM(void);
void SetupADCEpwm(Uint16 channel);

__interrupt void IRQ_EPWM1(void);
__interrupt void IRQ_ADC_A1(void);

#define BAUDRATE			230400U
#define	F_CPU				200000000U

#define LED_RD(x)			GPIO_writePin(31, x)
#define LED_BL(x)			GPIO_writePin(34, x)
#define IO_SET_DBG(PIN, x)	GPIO_writePin(PIN, x)
#define IO_TGL_DBG(PIN)		GPIO_togglePin(PIN)

#define RESULTS_BUFFER_SIZE 256

Uint16 adcaRes[RESULTS_BUFFER_SIZE];
Uint16 resultsIndex;

void initADC(const uint32_t ADCx_BASE, ADC_Resolution resolution, ADC_ClkPrescale clk_div);
void initEPWM(const uint32_t EPWMx_BASE, float samplePeriode,
              EPWM_ADCStartOfConversionType SOC_AB,
              EPWM_CounterCompareModule CounterCompareModule, uint16_t preScaleCount);
void configADC_SOC(const uint32_t ADCx_BASE, ADC_Channel CH_ADCINx,
                ADC_IntNumber INT_NUMBERx, ADC_SOCNumber SOCx, uint32_t sampleWindow_ns);
ADC_Resolution ADC_getResolution(const uint32_t ADCx_BASE);
ADC_SignalMode ADC_getSignalMode(const uint32_t ADCx_BASE);


/*
 * ===================== main of 06_adcEpwmSoc_dclClaPid =====================
 */
void main(void) {
	Device_init();				//!< Initialize device clock and peripherals
	Device_initGPIO();			//!< Disable pin locks and enable internal pullups.

#ifdef _FLASH
	//!< Initialize the FLASH
	memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (unsigned long)(&RamfuncsLoadSize));
	Flash_initModule(FLASH0CTRL_BASE, FLASH0ECC_BASE, DEVICE_FLASH_WAITSTATES);
#endif

	//!< Initialize PIE, clear PIE registers, disable CPU interrupts
	Interrupt_initModule();
	//!< Initialize the PIE vector table with pointers to the shell ISR'S
	Interrupt_initVectorTable();
	//!< Setup all predefined GPIOs from
	MD_GPIO_Setup(&MD_GPIO_config[0], GPIO_COUNT_IN_PACKED_INIT_STRUCT);
	//!< Configure SCIA for UART operation
	MD_SCIx_init(SCIA_BASE, BAUDRATE);

#ifdef MDV1
	//!< Configure ePWM module 1 for ??us period (sample frequency 1/100us)
	//!< Register ePWM1 interrupt callback.
	MD_EPWM1_init_mm(Ts * 1e6);
	Interrupt_register(INT_EPWM1, &IRQ_EPWM_1);

	//!< Configure ADCA module for ADC interrupt 1 and initialize the SOC to SOC0
	//!< Register ADCA1 EOC interrupt callback.
	MD_ADCA_init_mm(ADC_CH_ADCIN3, 75);
	Interrupt_register(INT_ADCA1, &IRQ_ADC_A1);

	EINT;
	//!< Enable Global interrupt INTM
	ERTM;
	//!< Enable Global realtime interrupt DBGM

	PieCtrlRegs.PIEIER1.bit.INTx1 = 1;//!< Enable PIE interrupt
	EALLOW;
	CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;//!< sync ePWM
	EDIS;

	//!< Start ePWM1 by putting the TBC into up-counting mode
	MD_EPWM1_acquisition(START);

	//!< Enable CPU interrupt groups
	Interrupt_enable(INT_ADCA1);
	Interrupt_enable(INT_EPWM1);
#else
	// Interrupts that are used in this example are re-mapped to ISR functions
	// found within this file.
	Interrupt_register(INT_ADCA1, &IRQ_ADC_A1);
	Interrupt_register(INT_EPWM1, &IRQ_EPWM1);

	// Set up the ADC and the ePWM and initialize the SOC
	initADC(ADCA_BASE, ADC_RESOLUTION_12BIT, ADC_CLK_DIV_4_0);
	initEPWM(EPWM1_BASE, Ts, EPWM_SOC_A, EPWM_COUNTER_COMPARE_A, 1);
	configADC_SOC(ADCA_BASE, ADC_CH_ADCIN3, ADC_INT_NUMBER1, ADC_SOC_NUMBER0, 75);
	Interrupt_enable(INT_ADCA1);	//!< Enable ADC interrupt
	Interrupt_enable(INT_EPWM1);	//!< Enable ADC interrupt

	// Enable Global Interrupt (INTM) and realtime interrupt (DBGM)
	EINT; ERTM;

	// Start ePWM1, enabling SOCA and putting counter in run mode
	EPWM_enableADCTrigger(EPWM1_BASE, EPWM_SOC_A);
	EPWM_setTimeBaseCounterMode(EPWM1_BASE, EPWM_COUNTER_MODE_UP_DOWN);
#endif

	while (1) {
		_delay_ms(250);
		MD_printi("ADCA1: ", adcRes[ADC_SOC_NUMBER0], " \n");
	}
}

/**
 * ADC_A Interrupt 1 ISR
 */
__interrupt void IRQ_ADC_A1(void) {
	//!< Add the latest result to the buffer
	adcRes[ADC_SOC_NUMBER0] = AdcaResultRegs.ADCRESULT0;

	//!< Clear INT1 flag and acknowledge PIE group 1
	AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}


/**
 * Function to initialize and power up ADCA.
 */
void initADC(const uint32_t ADCx_BASE, ADC_Resolution resolution, ADC_ClkPrescale clk_div) {
	// Set ADCCLK divider to /4
	ADC_setPrescaler(ADCx_BASE, clk_div);

	// Set resolution and signal mode (see #defines above) and load
	// corresponding trims.
	ADC_setMode(ADCx_BASE, resolution, ADC_MODE_SINGLE_ENDED);

	// Set pulse positions to late
	ADC_setInterruptPulseMode(ADCx_BASE, ADC_PULSE_END_OF_CONV);

	// Power up the ADC and then delay for 1 ms
	ADC_enableConverter(ADCx_BASE);
	DEVICE_DELAY_US(1000);
}

// Function to configure ePWM1 to generate the SOC.
void initEPWM(const uint32_t EPWMx_BASE, float samplePeriode,
              EPWM_ADCStartOfConversionType SOC_AB,
              EPWM_CounterCompareModule CounterCompareModule, uint16_t preScaleCount) {
	// Disable SOCA
	EPWM_disableADCTrigger(EPWMx_BASE, SOC_AB);

	// Configure the SOC to occur on the first up-count event
	EPWM_setADCTriggerSource(EPWMx_BASE, SOC_AB,
	        (SOC_AB == EPWM_SOC_A) ? EPWM_SOC_TBCTR_U_CMPA : EPWM_SOC_TBCTR_U_CMPB);

	EPWM_setADCTriggerEventPrescale(EPWMx_BASE, SOC_AB, preScaleCount);

	// Set the compare A value to 2048 and the period to 4096
	EPWM_setCounterCompareValue(EPWMx_BASE, CounterCompareModule,
	        _CMPA(1e6 * samplePeriode/10));

	EPWM_setTimeBasePeriod(EPWMx_BASE, _TBPRD(1e6 * samplePeriode));

	/**
	 * TBC run mode: UP_DOWN
	 * - Ignore event TBCTR == CMPA when in up counting direction
	 * - Do noting when TBCTR == TBPRD
	 * - Force EPWMxA pin high when TBCTR == CMPA in down counting direction
	 * - Force EPWMxA pin low when TBCTR == 0!
	 */
	EALLOW;
	EPwm1Regs.AQCTLA.bit.CAD = ACT_FORCE_HIGH;		//!< Event Compare A while down counting
	EPwm1Regs.AQCTLA.bit.CAU = ACT_IGNORE;			//!< Event Compare A while up counting
	EPwm1Regs.AQCTLA.bit.PRD = ACT_IGNORE;			//!< Event counter equals period
	EPwm1Regs.AQCTLA.bit.ZRO = ACT_FORCE_LOW;		//!< Event counter equals zero
	EDIS;

	EPWM_enableInterrupt(EPWMx_BASE);
	// Freeze the counter
	EPWM_setTimeBaseCounterMode(EPWMx_BASE, EPWM_COUNTER_MODE_STOP_FREEZE);
}

__interrupt void IRQ_EPWM1(void) {
	// Clear INT flag for this timer
	EPWM_clearEventTriggerInterruptFlag(EPWM1_BASE);

	IO_TGL_DBG(GPIO_DBG16);

	// Acknowledge interrupt group
	Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP3);
}

// Function to configure ADCA's SOC0 to be triggered by ePWM1.
void configADC_SOC(const uint32_t ADCx_BASE, ADC_Channel CH_ADCINx,
                ADC_IntNumber INT_NUMBERx, ADC_SOCNumber SOCx, uint32_t sampleWindow_ns) {

	//!< Assert sampleWindow_ns is modulo 5 divide-able then:
	//! sampleWindow 	= sampleWindow_ns / 5ns
	//! 				= sampleWindow_ns * 200 MHz
	ASSERT((sampleWindow_ns % 5) == 0);

	// Configure SOC0 of ADCA to convert pin A0. The EPWM1SOCA signal will be
	// the trigger.
	// For 12-bit resolution, a sampling window of 15 (75 ns at a 200MHz
	// SYSCLK rate) will be used.  For 16-bit resolution, a sampling window of
	// 64 (320 ns at a 200MHz SYSCLK rate) will be used.

	if (ADC_getResolution(ADCx_BASE) == ADC_RESOLUTION_12BIT) ADC_setupSOC(ADCx_BASE,
	        SOCx, ADC_TRIGGER_EPWM1_SOCA, CH_ADCINx, (uint32_t) sampleWindow_ns / 5);

	if (ADC_getResolution(ADCx_BASE) == ADC_RESOLUTION_16BIT) ADC_setupSOC(ADCx_BASE,
	        SOCx, ADC_TRIGGER_EPWM1_SOCA, CH_ADCINx,
	        (uint32_t) sampleWindow_ns * (float) 4 / 5);

	// Set SOC0 to set the interrupt 1 flag. Enable the interrupt and make
	// sure its flag is cleared.
	ADC_setInterruptSource(ADCx_BASE, INT_NUMBERx, SOCx);
	ADC_enableInterrupt(ADCx_BASE, INT_NUMBERx);
	ADC_clearInterruptStatus(ADCx_BASE, INT_NUMBERx);
}

ADC_Resolution ADC_getResolution(const uint32_t ADCx_BASE) {
	ASSERT(ADC_isBaseValid(ADCx_BASE));
	return (ADC_Resolution) (HWREGH(ADCx_BASE + ADC_O_CTL2) & ADC_CTL2_RESOLUTION);
}

ADC_SignalMode ADC_getSignalMode(const uint32_t ADCx_BASE) {
	ASSERT(ADC_isBaseValid(ADCx_BASE));
	return (ADC_SignalMode) (HWREGH(ADCx_BASE + ADC_O_CTL2) & ADC_CTL2_SIGNALMODE);
}

#ifdef INCLUDEME
void ConfigureADC(void) {
	EALLOW;

	//write configurations
	AdcaRegs.ADCCTL2.bit.PRESCALE = 6;  //set ADCCLK divider to /4
	AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);

	//Set pulse positions to late
	AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;

	//power up the ADC
	AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;

	//delay for 1ms to allow ADC time to power up
	DELAY_US(1000);

	EDIS;
}

// ConfigureEPWM - Configure EPWM SOC and compare values
void ConfigureEPWM(void) {
	EALLOW;
	// Assumes ePWM clock is already enabled
	EPwm1Regs.ETSEL.bit.SOCAEN = 0;    // Disable SOC on A group
	EPwm1Regs.ETSEL.bit.SOCASEL = 4;   // Select SOC on up-count
	EPwm1Regs.ETPS.bit.SOCAPRD = 1;       // Generate pulse on 1st event
	EPwm1Regs.CMPA.bit.CMPA = 0x0800;     // Set compare A value to 2048 counts
	EPwm1Regs.TBPRD = 0x1000;             // Set period to 4096 counts
	EPwm1Regs.TBCTL.bit.CTRMODE = 3;      // freeze counter
	EDIS;
}

// SetupADCEpwm - Setup ADC EPWM acquisition window
void SetupADCEpwm(Uint16 channel) {
	Uint16 acqps;

	//determine minimum acquisition window (in SYSCLKS) based on resolution
	if (ADC_RESOLUTION_12BIT == AdcaRegs.ADCCTL2.bit.RESOLUTION) {
		acqps = 14;  //75ns
	}
	else  //resolution is 16-bit
	{
		acqps = 63;  //320ns
	}

	//Select the channels to convert and end of conversion flag
	EALLOW;
	AdcaRegs.ADCSOC0CTL.bit.CHSEL = channel;  //SOC0 will convert pin A0
	AdcaRegs.ADCSOC0CTL.bit.ACQPS = acqps;  //sample window is 100 SYSCLK cycles
	AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 5;  //trigger on ePWM1 SOCA/C
	AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 0;  //end of SOC0 will set INT1 flag
	AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;   //enable INT1 flag
	AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;  //make sure INT1 flag is cleared
	EDIS;
}

void MD_ADCA_init_mm(ADC_Channel CH_ADCINx, uint32_t sampleWindow_ns) {
	/* Power-Up Sequence
	 * Upon device power-up or system level reset, the ADC will be powered down and disabled. When
	 * powering up the ADC, use the following sequence:
	 *
	 * 1. Set the bit to enable the desired ADC clock in the PCLKCR13 register.
	 * 2. Set the desired ADC clock divider in the PRESCALE field of ADCCTL2.
	 * 3. Power up the ADC by setting the ADCPWDNZ bit in ADCCTL1.
	 * 4. Allow a delay before sampling. See the data manual for the necessary time.
	 *
	 * If multiple ADCs are powered up simultaneously, steps 1 and step 3 can each be done for all ADCs in one
	 * write instruction. Also, only one delay is necessary as long as it occurs after all the ADCs have begun
	 * powering up
	 */

	EALLOW;
	AdcaRegs.ADCCTL2.bit.PRESCALE = ADC_CLK_DIV_4_0;
	AdcaRegs.ADCCTL2.bit.RESOLUTION = ADC_RESOLUTION_12BIT_bf;
	AdcaRegs.ADCCTL2.bit.SIGNALMODE = ADC_SIGNALMODE_SINGLE;
	ADC_setInterruptPulseMode(ADCA_BASE, ADC_PULSE_END_OF_CONV);
	AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;
	DELAY_US(1000);

	AdcaRegs.ADCSOC0CTL.bit.CHSEL = CH_ADCINx;
	AdcaRegs.ADCSOC0CTL.bit.ACQPS = (uint32_t) sampleWindow_ns / 5;
	AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 5;  //SOCx;  //trigger on ePWM1 SOCA/C

	AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 0;  //end of SOC0 will set INT1 flag
	AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;   //enable INT1 flag
	AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;  //make sure INT1 flag is cleared
	EDIS;
}

/**
 * Power up and configure ADC module, Configure SOCx of ADCx to convert pin CH_ADCINx.
 * The EPWM1SOCA signal will be the trigger.
 * @param ADCx_BASE
 * @param CH_ADCINx
 * @param INTx
 * @param SOCx
 * @param sampleWindow_ns
 */
void MD_ADC_init(const uint32_t ADCx_BASE, ADC_Channel CH_ADCINx, ADC_IntNumber INTx,
                 ADC_SOCNumber SOCx, uint32_t sampleWindow_ns) {
	//!< Assert sampleWindow_ns is modulo 5 divide-able then:
	//! sampleWindow 	= sampleWindow_ns / 5ns
	//! 				= sampleWindow_ns * 200 MHz
	ASSERT((sampleWindow_ns % 5) == 0);

	//!< Set ADCCLK divider to /4
	ADC_setPrescaler(ADCx_BASE, ADC_CLK_DIV_4_0);

	//!< Set resolution and signal mode (see #defines above) and load corresponding trims.
	ADC_setMode(ADCx_BASE, ADC_RESOLUTION_12BIT, ADC_MODE_SINGLE_ENDED);

	//!< Set SOCx to set the INTx flag.
	//! Enable the interrupt and make sure its flag is cleared.
	ADC_setInterruptSource(ADCx_BASE, INTx, SOCx);
	ADC_enableInterrupt(ADCx_BASE, INTx);
	ADC_clearInterruptStatus(ADCx_BASE, INTx);

	//!< For 12-bit resolution, a sampling window of 15 (75 ns at a 200MHz SYSCLK rate)
	//! will be used. For 16-bit resolution, a sampling window of 64 (320 ns at a
	//! 200MHz SYSCLK rate) will be used.
	ADC_setupSOC(ADCx_BASE, SOCx, ADC_TRIGGER_EPWM1_SOCA, CH_ADCINx,
	        (uint32_t) sampleWindow_ns / 5);

	//!< Set pulse positions to late
	ADC_setInterruptPulseMode(ADCx_BASE, ADC_PULSE_END_OF_CONV);

	//!< Power up the ADC and then delay for 1 ms
	ADC_enableConverter(ADCx_BASE);
	DEVICE_DELAY_US(1000);
}

/**
 * Configure DAC - Setup the reference voltage and output value for the DAC
 * @param DACx_BASE
 * @param initialOut shadow register value
 */
void MD_DAC_init(const uint32_t DACx_BASE, const uint16_t initialOut) {
	//!< Set VDAC as the DAC reference voltage.
	DAC_setReferenceVoltage(DACx_BASE, DAC_REF_ADC_VREFHI);

#ifdef EPWM_SIGNAL_LOAD_MODE
	DAC_setLoadMode(DACx_BASE, DAC_LOAD_PWMSYNC);
	DAC_setPWMSyncSignal(DACx_BASE, <PWM_sync_signal_no>);
#else
	DAC_setLoadMode(DACx_BASE, DAC_LOAD_SYSCLK);
#endif
	//!< Enable the DAC output
	DAC_enableOutput(DACx_BASE);

	//!< Set the initial DAC shadow output value
	DAC_setShadowValue(DACx_BASE, initialOut);

	//!< Delay for buffered DAC to power up
	DEVICE_DELAY_US(10);
}
#endif
