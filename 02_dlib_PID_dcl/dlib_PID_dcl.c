/**
 * @file        main.c
 * @project		dlib_bitfield_dcl
 *
 * @date        20 May 2017
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
#include "F28x_Project.h"
#include "F2837xD_GlobalPrototypes.h"
#include "device.h"
#include "driverlib.h"
#include "DCL.h"
#include <string.h>
#include <stdint.h>

#undef DEVICE_OSCSRC_FREQ
#define DEVICE_OSCSRC_FREQ 10000000U

#include "MD_F2837xD/md_gpio.h"
#include "md_gpio_conf.h"

typedef enum {
	EOL_LF = 0, EOL_NO
} EOL_t;

interrupt void control_Isr(void);

#ifdef BL_MODEL
void MD_InitDclExample_dl();
void MD_InitAdcs_dl(void);
void MD_InitEpwm_dl(const uint32_t sampleFreq);
void MD_SCI_jsonPuts(const char *key, const int32_t value);
#endif

void MD_puts(const char *str, EOL_t EOL);
char *int2str(const uint32_t number, char *buff);
void MD_InitSci(void);

void MD_ADC_config(void);
void MD_ADC_configSoc(void);
void MD_EPWM_config(const uint32_t sampleFreq);
void MD_DAC_config(uint16_t dac_num);
void MD_PID_instInit(PID *pidInst);
interrupt void ADC_EOC_Irq(void);

// global  variables
long IdleLoopCount = 0;
long IsrCount = 0;
float rk = 0.25f, yk, lk, uk;
PID pid1 = PID_DEFAULTS;
float Duty;

volatile struct DAC_REGS* DAC_PTR[4] =
		{ 0x0, &DacaRegs, &DacbRegs, &DaccRegs };

short dir = 1;
uint16_t dacVal = 2048;
//uint16_t DAC_VALS_DACVALS_M = 4095;
//uint16_t DAC_VALS_DACVALS_S = 1;
float uk_dac;

//
// Defines
//
#define REFERENCE_VDAC      0
#define REFERENCE_VREF      1
#define DACA         		1
#define DACB         		2
#define DACC         		3
#define REFERENCE           REFERENCE_VREF
#define DAC_NUM             DACA

#define MD_EPWM_UP_START()	EALLOW; \
		EPwm1Regs.TBCTL.bit.CTRMODE = EPWM_COUNTER_MODE_UP; \
		EDIS;

#define MD_EPWM_FREEZ()	EALLOW; \
		EPwm1Regs.TBCTL.bit.CTRMODE = EPWM_COUNTER_MODE_STOP_FREEZE; \
		EDIS;

volatile int loopCounter;

extern MD_GPIO_t GPIO_config[];

volatile int tfl;

#define FS 10e3

#define RESULTS_BUFFER_SIZE  256
#define EX_ADC_RESOLUTION    ADC_RESOLUTION_12BIT   // Or ADC_RESOLUTION_16BIT
#define EX_ADC_SIGNAL_MODE   ADC_MODE_SINGLE_ENDED  // Or ADC_MODE_DIFFERENTIAL

//
// Globals
//

//
// Function Prototypes
//
void initADC(const uint32_t ADCx_BASE);
void initEPWM(void);
void initADCSOC(const uint32_t ADCx_BASE, ADC_Channel adcChannel);
__interrupt void adcA1ISR(void);

uint16_t index;                              // Index into result buffer
volatile uint16_t bufferFull;                // Flag to indicate buffer is full

typedef uint16_t ResultsBuff[RESULTS_BUFFER_SIZE];   // Buffer for results

typedef struct {
	uint32_t 		BASE;
	ADC_Channel		AdcChan;
	ResultsBuff 	Buff;
	uint32_t		ResultBase;
} ADCx_t;

#define N_ADCS 3
ADCx_t ADCx[N_ADCS] = {
		{ .BASE = ADCA_BASE, .AdcChan = ADC_CH_ADCIN3, .ResultBase = ADCARESULT_BASE },
		{ .BASE = ADCB_BASE, .AdcChan = ADC_CH_ADCIN2, .ResultBase = ADCBRESULT_BASE },
		{ .BASE = ADCC_BASE, .AdcChan = ADC_CH_ADCIN2, .ResultBase = ADCCRESULT_BASE }
};

/*
 * ===================== main of _A_dlib_bitfield_dcl =====================
 */
void main(void) {
	//	char msgArr[64];

	char i2sArr[9];
	char *i2s = &i2sArr[0];

	// Configure PLL, disable WD, enable peripheral clocks.
	Device_init();

#ifdef _FLASH
	//!< Initialize the FLASH
	memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (unsigned long)(&RamfuncsLoadSize));
	InitFlash();
#endif

	// Disable pin locks and enable internal pullups.
	Device_initGPIO();
	MD_GPIO_Setup(&MD_GPIO_config[0], numel(MD_GPIO_config));

	// Initialize interrupt controller and vector table.
	Interrupt_initModule();
	Interrupt_initVectorTable();

	MD_InitSci();

	// Interrupts that are used in this example are re-mapped to ISR functions
	// found within this file.
	Interrupt_register(INT_ADCA1, &adcA1ISR);

	//	// Set up the ADC and the ePWM and initialize the SOC

	for (int n=0; n<N_ADCS; n++) {
		initADC(ADCx[n].BASE);
		initADCSOC(ADCx[n].BASE, ADCx[n].AdcChan);
	}
	initEPWM();

	//	MD_DAC_config(DAC_NUM);
	MD_PID_instInit(&pid1);

	Interrupt_enable(INT_ADCA1);				//!< Enable ADC interrupt
	Interrupt_enable(INT_ADCA2);				//!< Enable ADC interrupt
	Interrupt_enable(INT_ADCA3);				//!< Enable ADC interrupt

	EINT;	    // Enable Global Interrupt (INTM) and realtime interrupt (DBGM)
	ERTM;

	/* ------------------------------------------------------------------------- */

	// Start ePWM1, enabling SOCA and putting the counter in up-count mode
	EPWM_enableADCTrigger(EPWM1_BASE, EPWM_SOC_A);
	EPWM_setTimeBaseCounterMode(EPWM1_BASE, EPWM_COUNTER_MODE_UP);

	MD_puts("DLIB & BITFIELD & DCL test...\n", EOL_NO);

	/* idle loop */
	while (1) {
		if (bufferFull) {
			bufferFull = 0;

			for (int bl=0; bl<RESULTS_BUFFER_SIZE; bl++) {
				for (int n=0; n<N_ADCS; n++) {
					int2str(ADCx[n].Buff[bl], i2s);
					MD_puts(i2s, EOL_NO);
					MD_puts("\t\t", EOL_NO);
				}
				MD_puts("\n", EOL_NO);
			}
		}
	}  // while
}

// ADC A Interrupt 1 ISR
__interrupt void adcA1ISR(void) {
	// Add the latest result to the buffer
	for (int n=0; n<N_ADCS; n++) {
		ADCx[n].Buff[index++] =
				ADC_readResult(ADCx[n].ResultBase, ADC_SOC_NUMBER0);
		// Clear the interrupt flag and issue ACK
		ADC_clearInterruptStatus(ADCx[n].BASE, ADC_INT_NUMBER1);
	}

	// Set the bufferFull flag if the buffer is full
	if (RESULTS_BUFFER_SIZE <= index) {
		index = 0;
		bufferFull = 1;
	}

	Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}
// Function to configure and power up ADCA.
//
void initADC(const uint32_t ADCx_BASE) {
	// Set ADCCLK divider to /4
	ADC_setPrescaler(ADCx_BASE, ADC_CLK_DIV_4_0);

	// Set resolution and signal mode (see #defines above) and load
	// corresponding trims.
	ADC_setMode(ADCx_BASE, ADC_RESOLUTION_12BIT, ADC_MODE_SINGLE_ENDED);

	// Set pulse positions to late
	ADC_setInterruptPulseMode(ADCx_BASE, ADC_PULSE_END_OF_CONV);

	// Power up the ADC and then delay for 1 ms
	ADC_enableConverter(ADCx_BASE);
	DEVICE_DELAY_US(1000);
}

//
// Function to configure ePWM1 to generate the SOC.
//
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

//
// Function to configure ADCA's SOC0 to be triggered by ePWM1.
//
void initADCSOC(const uint32_t ADCx_BASE, ADC_Channel adcChannel) {
	// Configure SOC0 of ADCA to convert pin A0. The EPWM1SOCA signal will be
	// the trigger.
	// For 12-bit resolution, a sampling window of 15 (75 ns at a 200MHz
	// SYSCLK rate) will be used.  For 16-bit resolution, a sampling window of
	// 64 (320 ns at a 200MHz SYSCLK rate) will be used.
	ADC_setupSOC(ADCx_BASE, ADC_SOC_NUMBER0,
			ADC_TRIGGER_EPWM1_SOCA, adcChannel, 15);

	// Set SOC0 to set the interrupt 1 flag. Enable the interrupt and make
	// sure its flag is cleared.
	ADC_setInterruptSource(ADCx_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER0);
	ADC_enableInterrupt(ADCx_BASE, ADC_INT_NUMBER1);
	ADC_clearInterruptStatus(ADCx_BASE, ADC_INT_NUMBER1);
}

void MD_DAC_config(uint16_t dac_num) {
	EALLOW;
	DAC_PTR[dac_num]->DACCTL.bit.DACREFSEL = REFERENCE;
	DAC_PTR[dac_num]->DACOUTEN.bit.DACOUTEN = 1;
	DAC_PTR[dac_num]->DACVALS.all = 0;
	DELAY_US(10);  // Delay for buffered DAC to power up
	EDIS;
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

/*!
 Initialization sequence for SCI module.
 */
void MD_InitSci(void) {
	// GPIO43 is the SCI Rx pin.
	GPIO_setMasterCore(43, GPIO_CORE_CPU1);
	GPIO_setPinConfig(GPIO_43_SCIRXDA);
	GPIO_setDirectionMode(43, GPIO_DIR_MODE_IN);
	GPIO_setPadConfig(43, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(43, GPIO_QUAL_ASYNC);

	// GPIO42 is the SCI Tx pin.
	GPIO_setMasterCore(42, GPIO_CORE_CPU1);
	GPIO_setPinConfig(GPIO_42_SCITXDA);
	GPIO_setDirectionMode(42, GPIO_DIR_MODE_OUT);
	GPIO_setPadConfig(42, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(42, GPIO_QUAL_ASYNC);

	// Initialize SCIA and its FIFO.
	SCI_performSoftwareReset(SCIA_BASE);

	// Configure SCIA for echoback.
	SCI_setConfig(SCIA_BASE, DEVICE_LSPCLK_FREQ, 115200, SCI_CONFIG_WLEN_8 |
	              SCI_CONFIG_STOP_ONE | SCI_CONFIG_PAR_NONE);
	SCI_resetChannels(SCIA_BASE);
	SCI_resetRxFIFO(SCIA_BASE);
	SCI_resetTxFIFO(SCIA_BASE);
	SCI_clearInterruptStatus(SCIA_BASE, SCI_INT_TXFF | SCI_INT_RXFF);
	SCI_enableFIFO(SCIA_BASE);
	SCI_enableModule(SCIA_BASE);
	SCI_performSoftwareReset(SCIA_BASE);

#ifdef AUTOBAUD
	// Perform an autobaud lock.
	// SCI expects an 'a' or 'A' to lock the baud rate.
	SCI_lockAutobaud(SCIA_BASE);
#endif

}

void MD_puts(const char *str, EOL_t EOL) {
	while (*str != '\0')
		SCI_writeCharBlockingNonFIFO(SCIA_BASE, (uint16_t) *str++);
	if (EOL == EOL_LF) SCI_writeCharBlockingFIFO(SCIA_BASE, (uint16_t) '\n');
}

#ifdef BF_MODEL
/*!
 Initialization sequence for EPWM modules.
 */
void MD_InitEpwm_dl(const uint32_t sampleFreq) {
	uint32_t base = EPWM1_BASE;

	SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);
	SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM1);
	/* configure EPWM */
	// freeze TB counter        // EPwm1Regs.TBCTL.bit.CTRMODE = 3;
	EPWM_setTimeBaseCounterMode(base, EPWM_COUNTER_MODE_STOP_FREEZE);
	// immediate load           // EPwm1Regs.TBCTL.bit.PRDLD = 1;
	EPWM_setPeriodLoadMode(base, EPWM_PERIOD_DIRECT_LOAD);
	// disable phase loading    // EPwm1Regs.TBCTL.bit.PHSEN = 0;
	EPWM_disablePhaseShiftLoad(base);
	// disable SYNCOUT signal   //
	EPWM_setSyncOutPulseMode(base, EPWM_SYNC_OUT_PULSE_DISABLED);
	// TBCLK = SYSCLKOUT        // EPwm1Regs.TBCTL.bit.HSPCLKDIV = 0;
	// clock divider = /1       // EPwm1Regs.TBCTL.bit.CLKDIV = 0;
	EPWM_setClockPrescaler(base, EPWM_CLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_1);
	// free run on emulation suspend            // EPwm1Regs.TBCTL.bit.FREE_SOFT = 2;
	EPWM_setEmulationMode(base, EPWM_EMULATION_FREE_RUN);
	// set period for ePWM1 (0x2328 = 10kHz)    // EPwm1Regs.TBPRD = 0x2328;
	volatile uint32_t SAMP = (uint32_t) SysCtl_getClock(DEVICE_OSCSRC_FREQ)/sampleFreq;
	EPWM_setTimeBasePeriod(base, SAMP);
	// time-base Phase Register                 // EPwm1Regs.TBPHS.all = 0;
	EPWM_setPhaseShift(base, 0);
	// time-base Counter Register               // EPwm1Regs.TBCTR = 0;
	EPWM_setTimeBaseCounter(base, 0);
	// enable SOC on A group                    // EPwm1Regs.ETSEL.bit.SOCAEN = 1;
	// select SOC from zero match               // EPwm1Regs.ETSEL.bit.SOCASEL = 1;
	EPWM_setADCTriggerSource(base, EPWM_SOC_A, EPWM_SOC_TBCTR_ZERO);
	// generate pulse on 1st event              // EPwm1Regs.ETPS.bit.SOCAPRD = 1;
	EPWM_setDigitalCompareBlankingEvent(base, EPWM_DC_WINDOW_START_TBCTR_ZERO);
	// enable shadow mode                       // EPwm1Regs.CMPCTL.bit.SHDWAMODE = 0;
	// reload on CTR=zero                       // EPwm1Regs.CMPCTL.bit.LOADAMODE = 2;
	EPWM_setCounterCompareShadowLoadMode(base, EPWM_COUNTER_COMPARE_A,
	                                     EPWM_COMP_LOAD_ON_SYNC_CNTR_ZERO);
	// set compare A value                      // EPwm1Regs.CMPA.all = 0x0080;
	EPWM_setCounterCompareValue(base, EPWM_COUNTER_COMPARE_A, 0x80);
	// HIGH on CMPA up match                    // EPwm1Regs.AQCTLA.bit.CAU = AQ_SET;
	EPWM_setActionQualifierAction(base, EPWM_AQ_OUTPUT_A,
	                              EPWM_AQ_OUTPUT_HIGH,
	                              EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
	// EPwm1Regs.AQCTLA.bit.ZRO = AQ_CLEAR;     // LOW on zero match
	EPWM_setActionQualifierActionComplete(base, EPWM_AQ_OUTPUT_A,
	                                      EPWM_AQ_OUTPUT_LOW_ZERO);

	SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

	EPwm1Regs.AQCTLA.bit.CAU = AQ_SET;// HIGH on CMPA up match
	EPwm1Regs.CMPCTL.bit.LOADAMODE = 2;// reload on CTR=zero
	EPwm1Regs.ETSEL.bit.SOCAEN = 1;// enable SOC on A group
	EPwm1Regs.ETPS.bit.SOCAPRD = 1;// generate pulse on 1st event

	MD_EPWM_UP_START();
}
/*!
 Initialization sequence for ADC modules.
 */
void MD_InitAdcs_dl(void) {
	uint32_t base = ADCA_BASE;

	/* configure ADC */
	SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_ADCA);
	ADC_enableConverter(base);
	// EALLOW;
	// early interrupt generation	// AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 0
	ADC_setInterruptPulseMode(base, ADC_PULSE_END_OF_ACQ_WIN);

	// enabled ADCINT1	// AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1
	// disable ADCINT1 continuous mode	// AdcaRegs.ADCINTSEL1N2.bit.INT1CONT = 0
	// setup EOC1 to trigger ADCINT1	// AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 1
	ADC_enableInterrupt(base, ADC_INT_NUMBER1);
	ADC_disableContinuousMode(base, ADC_INT_NUMBER1);
	ADC_setInterruptSource(base, ADC_INT_NUMBER1, ADC_SOC_NUMBER1);
	ADC_setInterruptSOCTrigger(base, ADC_SOC_NUMBER1, ADC_INT_SOC_TRIGGER_ADCINT1);

	// enable ADCINT2	// AdcaRegs.ADCINTSEL1N2.bit.INT2E = 0
	// disable ADCINT2 continuous mode	// AdcaRegs.ADCINTSEL1N2.bit.INT2CONT = 0
	// setup EOC1 to trigger ADCINT2	// AdcaRegs.ADCINTSEL1N2.bit.INT2SEL = 0
	ADC_enableInterrupt(base, ADC_INT_NUMBER2);
	ADC_disableContinuousMode(base, ADC_INT_NUMBER2);
	ADC_setInterruptSource(base, ADC_INT_NUMBER2, ADC_SOC_NUMBER1);
	ADC_setInterruptSOCTrigger(base, ADC_SOC_NUMBER1, ADC_INT_SOC_TRIGGER_ADCINT2);

	// set SOC0 channel select to ADCINA0	// AdcaRegs.ADCSOC0CTL.bit.CHSEL = 0
	// set SOC0 start trigger on EPWM1A, due to round-robin SOC0 converts first then SOC1	// AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 5
	// set SOC0 S/H Window to 7 ADC Clock Cycles, (6 ACQPS plus 1)	// AdcaRegs.ADCSOC0CTL.bit.ACQPS = 6
	ADC_setupSOC(base, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN0, 7);

	// set SOC1 channel select to ADCINB0	// AdcaRegs.ADCSOC1CTL.bit.CHSEL = 8
	// set SOC1 start trigger on EPWM1A, due to round-robin SOC0 converts first then SOC1	// AdcaRegs.ADCSOC1CTL.bit.TRIGSEL = 5
	// set SOC1 S/H Window to 7 ADC Clock Cycles, (6 ACQPS plus 1)	// AdcaRegs.ADCSOC1CTL.bit.ACQPS = 6
	ADC_setupSOC(base, ADC_SOC_NUMBER1, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN8, 7);

}
/*!
 Initialization sequence from DCL example (Driverlib programming model).
 */
void MD_InitDclExample_dl(void) {

	/* initialise controller variables */
	pid1.Kp = 9.0f;
	pid1.Ki = 0.015f;
	pid1.Kd = 0.35f;
	pid1.Kr = 1.0f;
	pid1.c1 = 188.0296600613396f;
	pid1.c2 = 0.880296600613396f;
	pid1.d2 = 0.0f;
	pid1.d3 = 0.0f;
	pid1.i10 = 0.0f;
	pid1.i14 = 1.0f;
	pid1.Umax = 1.0f;
	pid1.Umin = -1.0f;

	rk = 0.25f;		// initial value for control reference
	lk = 1.0f;// control loop not saturated

	MD_EPWM_UP_START();
}
/*!
 Initialization sequence from DCL example (Bitfield programming model).
 */
#endif

/**
 * @brief      Simple digit to char converter.
 *
 * @param      number   The number
 * @param[in]  base     The base
 * @return     character representation of number
 */
char makedigit(uint32_t *number, uint32_t base) {
	static char map[] = "0123456789";
	uint16_t ix;

	for (ix = 0; *number >= base; ix++)
		*number -= base;

	return map[ix];
}

/**
 * @brief      Simple integral to string converter.
 *
 * @param[in]  number   The number
 * @return     Pointer to string representation of number.
 */
char *int2str(const uint32_t number, char *buff) {
	//	static char tmp[12] = { " " };
	//	char *p = &tmp[0];
	uint32_t _number = number;
	bool firstNonzero = false;

	if (!_number) {
		*buff++ = '0';
		*buff = '\0';
		return buff;
	}

	for (uint32_t ddec = 1e9; ddec >= 1; ddec /= 10) {
		*buff = makedigit(&_number, ddec);
		if ((*buff != '0') || firstNonzero) {
			firstNonzero = true;
			buff++;
		}
	}
	*buff = '\0';
	return buff;
}

/*!
 Control ISR: triggered by ADC EOC
 */
interrupt void control_Isr(void) {
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
	AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;

	// read ADC channel
	yk = ((float) AdcaResultRegs.ADCRESULT0 - 2048.0f) / 2047.0f;
	lk = (float) AdcaResultRegs.ADCRESULT1;

	// external clamp for anti-windup reset
	DCL_runClamp_C1(&lk, 1.0f, 0.0f);

	// run PID controller
	uk = DCL_runPID_C4(&pid1, rk, yk, lk);

	// write u(k) to PWM
	//	Duty = (uk / 2.0f + 0.5f) * (float) EPwm1Regs.TBPRD;
	//	EPwm1Regs.CMPA.bit.CMPA = (Uint16) EPwm1Regs.TBPRD/4;

	//	GPIO_togglePin(MD_GPIO_config[3].Gpiox);
	IsrCount++;
}

#ifdef OLD
/* control ISR: triggered by ADC EOC */
interrupt void ADC_EOC_Irq(void) {
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
	AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;

	// read ADC channel
	yk = ((float) AdcbResultRegs.ADCRESULT0 - 2048.0f) / 2047.0f;
	rk = ((float) AdccResultRegs.ADCRESULT0 - 2048.0f) / 2047.0f;
	lk = 1.0f;

	DCL_runClamp_C1(&lk, 1.0f, 0.0f);//!< external clamp for anti-windup reset
	uk = DCL_runPID_C4(&pid1, rk, yk, lk);//!< run PID controller

	uk_dac = (uk / 2.0f + 0.5f) * (float) DAC_VALS_DACVALS_M;//!< write u(k) to PWM
	//	DAC_PTR[DAC_NUM]->DACVALS.all = (rk / 2.0f + 0.5f) * (float) DAC_VALS_DACVALS_M;
	DAC_PTR[DAC_NUM]->DACVALS.all = uk_dac;

	//	EPwm1Regs.CMPA.all  = (Uint16) Duty;
	IsrCount++;
}

void MD_EPWM_config(const uint32_t sampleFreq) {
	EALLOW;
	EPwm1Regs.TBCTL.bit.CTRMODE = 3;      // freeze counter
	EPwm1Regs.ETSEL.bit.SOCAEN = 0;// Disable SOC on A group
	EPwm1Regs.ETSEL.bit.SOCASEL = 4;// Select SOC on up-count
	EPwm1Regs.ETPS.bit.SOCAPRD = 1;// Generate pulse on 1st event
	volatile uint32_t SAMP = (uint32_t) SysCtl_getClock(DEVICE_OSCSRC_FREQ)/sampleFreq;

	EPwm1Regs.TBPRD = SAMP;// Set period to 4096 counts
	EPwm1Regs.CMPA.bit.CMPA = 0.5 * SAMP;// Set compare A value to 2048 counts
	EDIS;
}

void MD_ADC_configSoc(void) {
	EALLOW;
	AdcaRegs.ADCSOC0CTL.bit.CHSEL = 3;  //SOC0 will convert pin A0
	AdcaRegs.ADCSOC0CTL.bit.ACQPS = 19;//sample window is 100 SYSCLK cycles
	AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 5;//trigger on ePWM1 SOCA/C

	AdcbRegs.ADCSOC0CTL.bit.CHSEL = 2;//SOC0 will convert pin A0
	AdcbRegs.ADCSOC0CTL.bit.ACQPS = 19;//sample window is 100 SYSCLK cycles
	AdcbRegs.ADCSOC0CTL.bit.TRIGSEL = 5;//trigger on ePWM1 SOCA/C

	AdccRegs.ADCSOC0CTL.bit.CHSEL = 1;//SOC0 will convert pin A0
	AdccRegs.ADCSOC0CTL.bit.ACQPS = 19;//sample window is 100 SYSCLK cycles
	AdccRegs.ADCSOC0CTL.bit.TRIGSEL = 5;//trigger on ePWM1 SOCA/C

	AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 0;//end of SOC0 will set INT1 flag
	AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;//enable INT1 flag
	AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;//make sure INT1 flag is cleared

	PieVectTable.ADCA1_INT = &ADC_EOC_Irq;//!< function for ADCA interrupt 1
	EDIS;

	IER |= M_INT1;//!< Enable group 1 interrupts
	PieCtrlRegs.PIEIER1.bit.INTx1 = 1;//!< Enable PIE interrupt
}

void MD_ADC_config(void) {
	EALLOW;
	AdcaRegs.ADCCTL2.bit.PRESCALE = 6;  //set ADCCLK divider to /4
	AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
	AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;
	AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;
	DELAY_US(1000);

	AdcbRegs.ADCCTL2.bit.PRESCALE = 6;//set ADCCLK divider to /4
	AdcSetMode(ADC_ADCB, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
	AdcbRegs.ADCCTL1.bit.INTPULSEPOS = 1;
	AdcbRegs.ADCCTL1.bit.ADCPWDNZ = 1;
	DELAY_US(1000);

	AdccRegs.ADCCTL2.bit.PRESCALE = 6;//set ADCCLK divider to /4
	AdcSetMode(ADC_ADCC, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
	AdccRegs.ADCCTL1.bit.INTPULSEPOS = 1;
	AdccRegs.ADCCTL1.bit.ADCPWDNZ = 1;
	DELAY_US(1000);
	EDIS;
}
#endif
