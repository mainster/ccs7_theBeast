/**
 * @file        main.c
 * @project		_D_adc_ex2_soc_epwm
 *
 * @date        23 May 2017
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
#include <main.h>
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
#include "md_globals.h"
#include "md_config.h"
#include "md_gpio.h"
#include "md_sci.h"

volatile struct DAC_REGS* DAC_PTR[4] =
	{ 0x0, &DacaRegs, &DacbRegs, &DaccRegs };

Uint16 dacVal = 2048;
short dir = 1;

PID pid1 = PID_DEFAULTS;	//!< PID instance

/*
 * ===================== main of _C_adc_ex2_soc_epwm =====================
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

	//!< Register IRQ callback handlers.
	Interrupt_register(INT_ADCA1, &IRQ_ADC_A1);
	Interrupt_register(INT_EPWM1, &IRQ_EPWM_1);

	//!< Setup all predefined GPIOs from
	MD_GPIO_Setup(&MD_GPIO_config[0], GPIO_COUNT_IN_PACKED_INIT_STRUCT);

	uint32_t ts = 100;
		//!< Configure ePWM module 1 for 100us period (sample frequency 1/100us)
	MD_EPWM_init(EPWM1_BASE, ts);

	//!< Configure ADCA module for ADC interrupt 1 and initialize the SOC to SOC0
	MD_ADC_init(ADCA_BASE, ADC_CH_ADCIN3, ADC_INT_NUMBER1, ADC_SOC_NUMBER0, 75);

	//!< Configure SCIA for UART operation
	MD_SCIx_init(SCIA_BASE, BAUDRATE);

	Interrupt_enable(INT_EPWM1);
	Interrupt_enable(INT_ADCA1);

	//!< Start ePWM1, enabling SOCA trigger and putting the counter in up-count mode
	EPWM_enableADCTrigger(EPWM1_BASE, EPWM_SOC_A);
	EPWM_setTimeBaseCounterMode(EPWM1_BASE, EPWM_COUNTER_MODE_UP);

	//!< Enable Global Interrupt (INTM) and realtime interrupt (DBGM).
	EINT; ERTM;

	while (1) {
//		MD_printi("pref: ", gc_slider, " post\n");
		DEVICE_DELAY_US(200000);
	}
}

/**
 *  EPWM_1 interrupt handler
 */
__interrupt void IRQ_EPWM_1(void) {
	// Clear INT flag for this timer
	EPWM_clearEventTriggerInterruptFlag(EPWM1_BASE);

	// Acknowledge interrupt group
	Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP3);
}

/**
 * ADC_A Interrupt 1 ISR
 */
__interrupt void IRQ_ADC_A1(void) {
	IO_TGL_DBG(16);

	// Add the latest result to the buffer
	adcAResults[index++] = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER0);

	// Set the bufferFull flag if the buffer is full
	if (RESULTS_BUFFER_SIZE <= index) {
		index = 0;
		bufferFull = 1;
	}

	// Clear the interrupt flag and issue ACK
	ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
	Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

/**
 * ADC_B Interrupt 2 ISR
 */
__interrupt void adcB2ISR(void) {
	// Add the latest result to the buffer
	adcAResults[index++] = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER1);

	// Set the bufferFull flag if the buffer is full
	if (RESULTS_BUFFER_SIZE <= index) {
		index = 0;
		bufferFull = 1;
	}

	// Clear the interrupt flag and issue ACK
	ADC_clearInterruptStatus(ADCB_BASE, ADC_INT_NUMBER2);
	Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
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
 * Initialize EPWM module and configure ADC trigger source.
 *
 * @param EPWMx_BASE
 * @param Ts_us
 */
void MD_EPWM_init(const uint32_t EPWMx_BASE, const uint32_t samplePeriod_us) {
	//!< Freeze the counter
	EPWM_setTimeBaseCounterMode(EPWMx_BASE, EPWM_COUNTER_MODE_STOP_FREEZE);
	SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

	//!< Disable SOCA trigger
	EPWM_disableADCTrigger(EPWMx_BASE, EPWM_SOC_A);

	//!< Configure the SOC to occur on the first up-count event
	EPWM_setADCTriggerSource(EPWMx_BASE, EPWM_SOC_A, EPWM_SOC_TBCTR_U_CMPA);
	EPWM_setADCTriggerEventPrescale(EPWMx_BASE, EPWM_SOC_A, 1);

	//!< Calculate time base period value
	const uint32_t reload = samplePeriod_us * ((uint32_t) F_CPU / 1000000);

	//!< Set the compare A value to half the calculated period value
	EPWM_setCounterCompareValue(EPWMx_BASE, EPWM_COUNTER_COMPARE_A, (reload >> 1));
	EPWM_setTimeBasePeriod(EPWMx_BASE, reload);

	//!< Set up Action qualifier output: Set output pin high when time base counter equals period.
	EPWM_setActionQualifierAction(EPWMx_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_HIGH,
	        EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);

	//!< Clear output pin when time base counter equals zero.
	EPWM_setActionQualifierActionComplete(EPWMx_BASE, EPWM_AQ_OUTPUT_A,
	        EPWM_AQ_OUTPUT_LOW_ZERO);

	//!< Trigger ePWM interrupt when time-base counter equals zero
	EPWM_setInterruptSource(EPWMx_BASE, EPWM_INT_TBCTR_ZERO);

	//!< Enable ePWM interrupt.
	EPWM_enableInterrupt(EPWMx_BASE);

	SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);
}

void MD_putKeyVal(const char *key, const char *val) {
	SCI_writeCharBlockingFIFO(SCIA_BASE, (uint16_t) '{');
	SCI_writeCharBlockingFIFO(SCIA_BASE, (uint16_t) '\"');
	MD_puts(key, EOL_NO);
	SCI_writeCharBlockingFIFO(SCIA_BASE, (uint16_t) '\"');
	SCI_writeCharBlockingFIFO(SCIA_BASE, (uint16_t) ':');
	MD_puts(val, EOL_NO);
	SCI_writeCharBlockingFIFO(SCIA_BASE, (uint16_t) '}');
	SCI_writeCharBlockingFIFO(SCIA_BASE, (uint16_t) '\n');
}

void MD_PID_instInit(PID *pidInst) {
	/* Initialize controller variables */
	pidInst->Kp = 0.8f;
	pidInst->Ki = 0.1f;
	pidInst->Kd = 0.0f;
	pidInst->Kr = 1.0f;
	pidInst->c1 = 1.0296600613396f;
	pidInst->c2 = 0.880296600613396f;
	pidInst->d2 = 0.0f;
	pidInst->d3 = 0.0f;
	pidInst->i10 = 0.0f;
	pidInst->i14 = 1.0f;
	pidInst->Umax = 1.0f;
	pidInst->Umin = -1.0f;
}
