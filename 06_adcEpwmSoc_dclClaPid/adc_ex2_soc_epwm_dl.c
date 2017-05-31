//#############################################################################
//
// FILE:   adc_ex1_soc_epwm.c
// TITLE:  ADC ePWM Triggering
//! \addtogroup driver_example_list
//! <h1>ADC ePWM Triggering</h1>
//!
//! This example sets up ePWM1 to periodically trigger a conversion on ADCA.
//!
//! \b External \b Connections \n
//!  - A0 should be connected to a signal to convert
//!
//! \b Watch \b Variables \n
//! - \b adcAResults - A sequence of analog-to-digital conversion samples from
//!   pin A0. The time between samples is determined based on the period
//!   of the ePWM timer.
//!
//#############################################################################
// $TI Release: F2837xD Support Library v3.01.00.00 $
// $Release Date: Mon May 22 15:43:40 CDT 2017 $
// $Copyright:
// Copyright (C) 2013-2017 Texas Instruments Incorporated - http://www.ti.com/
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions 
// are met:
// 
//   Redistributions of source code must retain the above copyright 
//   notice, this list of conditions and the following disclaimer.
// 
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the 
//   documentation and/or other materials provided with the   
//   distribution.
// 
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// $
//#############################################################################

// Included Files
#include "driverlib.h"
#include "device.h"

// Defines
#define RESULTS_BUFFER_SIZE  256
#define EX_ADC_RESOLUTION    ADC_RESOLUTION_12BIT   // Or ADC_RESOLUTION_16BIT
#define EX_ADC_SIGNAL_MODE   ADC_MODE_SINGLE_ENDED  // Or ADC_MODE_DIFFERENTIAL

// Globals
uint16_t adcAResults[RESULTS_BUFFER_SIZE];   // Buffer for results
uint16_t index;                              // Index into result buffer
volatile uint16_t bufferFull;                // Flag to indicate buffer is full

// Function Prototypes
void initADC(void);
void initEPWM(void);
void initADCSOC(void);
__interrupt void adcA1ISR(void);

// Main
void main(void) {
	// Initialize device clock and peripherals
	Device_init();

	// Disable pin locks and enable internal pullups.
	Device_initGPIO();

	// Initialize PIE and clear PIE registers. Disables CPU interrupts.
	Interrupt_initModule();

	// Initialize the PIE vector table with pointers to the shell Interrupt
	// Service Routines (ISR).
	Interrupt_initVectorTable();

	// Interrupts that are used in this example are re-mapped to ISR functions
	// found within this file.
	Interrupt_register(INT_ADCA1, &adcA1ISR);

	// Set up the ADC and the ePWM and initialize the SOC
	initADC();
	initEPWM();
	initADCSOC();

	// Initialize results buffer
	for (index = 0; index < RESULTS_BUFFER_SIZE; index++) {
		adcAResults[index] = 0;
	}

	index = 0;
	bufferFull = 0;

	// Enable ADC interrupt
	Interrupt_enable(INT_ADCA1);

	// Enable Global Interrupt (INTM) and realtime interrupt (DBGM)
	EINT;
	ERTM;

	// Loop indefinitely
	while (1) {
		// Start ePWM1, enabling SOCA and putting the counter in up-count mode
		EPWM_enableADCTrigger(EPWM1_BASE, EPWM_SOC_A);
		EPWM_setTimeBaseCounterMode(EPWM1_BASE, EPWM_COUNTER_MODE_UP);

		// Wait while ePWM1 causes ADC conversions which then cause interrupts.
		// When the results buffer is filled, the bufferFull flag will be set.
		while (bufferFull == 0) {
		}
		bufferFull = 0;     // Clear the buffer full flag

		// Stop ePWM1, disabling SOCA and freezing the counter
		EPWM_disableADCTrigger(EPWM1_BASE, EPWM_SOC_A);
		EPWM_setTimeBaseCounterMode(EPWM1_BASE, EPWM_COUNTER_MODE_STOP_FREEZE);

		// Software breakpoint. At this point, conversion results are stored in
		// adcAResults.
		// Hit run again to get updated conversions.
		ESTOP0;
	}
}

// Function to configure and power up ADCA.
void initADC(void) {
	// Set ADCCLK divider to /4
	ADC_setPrescaler(ADCA_BASE, ADC_CLK_DIV_4_0);

	// Set resolution and signal mode (see #defines above) and load
	// corresponding trims.
	ADC_setMode(ADCA_BASE, EX_ADC_RESOLUTION, EX_ADC_SIGNAL_MODE);

	// Set pulse positions to late
	ADC_setInterruptPulseMode(ADCA_BASE, ADC_PULSE_END_OF_CONV);

	// Power up the ADC and then delay for 1 ms
	ADC_enableConverter(ADCA_BASE);
	DEVICE_DELAY_US(1000);
}

// Function to configure ePWM1 to generate the SOC.
void initEPWM(void) {
	// Disable SOCA
	EPWM_disableADCTrigger(EPWM1_BASE, EPWM_SOC_A);

	// Configure the SOC to occur on the first up-count event
	EPWM_setADCTriggerSource(EPWM1_BASE, EPWM_SOC_A, EPWM_SOC_TBCTR_U_CMPA);
	EPWM_setADCTriggerEventPrescale(EPWM1_BASE, EPWM_SOC_A, 1);

	// Set the compare A value to 2048 and the period to 4096
	EPWM_setCounterCompareValue(EPWM1_BASE, EPWM_COUNTER_COMPARE_A, 0x0800);
	EPWM_setTimeBasePeriod(EPWM1_BASE, 0x1000);

	// Freeze the counter
	EPWM_setTimeBaseCounterMode(EPWM1_BASE, EPWM_COUNTER_MODE_STOP_FREEZE);
}

// Function to configure ADCA's SOC0 to be triggered by ePWM1.
void initADCSOC(void) {
	// Configure SOC0 of ADCA to convert pin A0. The EPWM1SOCA signal will be
	// the trigger.
	// For 12-bit resolution, a sampling window of 15 (75 ns at a 200MHz
	// SYSCLK rate) will be used.  For 16-bit resolution, a sampling window of
	// 64 (320 ns at a 200MHz SYSCLK rate) will be used.
#if(EX_ADC_RESOLUTION == ADC_RESOLUTION_12BIT)
	ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN0, 15);
#elif(EX_ADC_RESOLUTION == ADC_RESOLUTION_16BIT)
	ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM1_SOCA,
			ADC_CH_ADCIN0, 64);
#endif

	// Set SOC0 to set the interrupt 1 flag. Enable the interrupt and make
	// sure its flag is cleared.
	ADC_setInterruptSource(ADCA_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER0);
	ADC_enableInterrupt(ADCA_BASE, ADC_INT_NUMBER1);
	ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
}

// ADC A Interrupt 1 ISR
__interrupt void adcA1ISR(void) {
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
