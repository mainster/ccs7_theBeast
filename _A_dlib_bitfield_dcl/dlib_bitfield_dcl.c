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

//!< TI memory mapped bit-field includes
#include "F28x_Project.h"
#include "F2837xD_GlobalPrototypes.h"

//!< TI driverlib includes
#include "device.h"
#include "driverlib.h"

#undef DEVICE_OSCSRC_FREQ
#define DEVICE_OSCSRC_FREQ 10000000U
#include "md_globals.h"
#include "md_gpio.h"
#include "md_gpio_conf.h"

//!< TI Digital Controller Library includes
#include "DCL.h"
#include <string.h>

typedef enum {
	EOL_LF = 0,
	EOL_NO
} EOL_t;

interrupt void control_Isr(void);
void MD_InitDclExample_bf(void);
void MD_InitDclExample_dl(const uint32_t sampleFreq);
void MD_InitAdcs(void);
void MD_InitEpwm(const uint32_t sampleFreq);
void MD_InitSci(void);

void MD_InitDclExample_bf(void);
void MD_InitSci(void);

char *int2str (const uint32_t number, char *buff);
void SCI_writeCharArrayLF(uint32_t base, char *array,
                          uint16_t length);
void MD_puts(const char *str, EOL_t EOL);

// global  variables
long IdleLoopCount = 0;
long IsrCount = 0;
float rk = 0.25f, yk, lk, uk;
PID pid1 = PID_DEFAULTS;
float Duty;

#define MD_EPWM_UP_START()	EALLOW; \
		EPwm1Regs.TBCTL.bit.CTRMODE = EPWM_COUNTER_MODE_UP; \
		EDIS;

#define MD_EPWM_FREEZ()	EALLOW; \
		EPwm1Regs.TBCTL.bit.CTRMODE = EPWM_COUNTER_MODE_STOP_FREEZE; \
		EDIS;

volatile int loopCounter;

extern MD_GPIO_t GPIO_config[];

volatile int tfl;

/*
 * ===================== main of _A_dlib_bitfield_dcl =====================
 */
void main(void) {
	char msgArr[64];
	char *msg = &msgArr[0];

	char i2sArr[9];
	char *i2s = &i2sArr[0];

	// Configure PLL, disable WD, enable peripheral clocks.
	Device_init();

	// Disable pin locks and enable internal pullups.
	Device_initGPIO();
	MD_GPIO_Setup(&MD_GPIO_config[0], numel(MD_GPIO_config));

	// Initialize interrupt controller and vector table.
	Interrupt_initModule();
	Interrupt_initVectorTable();

	MD_InitSci();

#ifdef AUTOBAUD
	// Perform an autobaud lock.
	// SCI expects an 'a' or 'A' to lock the baud rate.
	SCI_lockAutobaud(SCIA_BASE);
#endif

	strcpy(msg, "DLIB & BITFIELD & DCL test...\n");
	SCI_writeCharArray(SCIA_BASE, (uint16_t*) msg, 62);

//	MD_InitDclExample_dl(10e3);
	MD_InitEpwm(10e3);
	MD_EPWM_UP_START();
	Interrupt_enableMaster();

	int2str(11, i2s);
	int2str(1541, i2s);
	int2str(31541, i2s);

	strcpy(msg, "Loop count: ");
	volatile int idx = numel(msgArr);

	/* idle loop */
	while (1) {
		IdleLoopCount++;					// increment loop counter
		int2str(IdleLoopCount, i2s);

		MD_puts(msg, EOL_NO);
		MD_puts(i2s, EOL_LF);

		DELAY_US(1000*100);
	}  // while

//
//	for (;;) {
//		msg = "\r\nEnter a character: \0";
//		SCI_writeCharArray(SCIA_BASE, (uint16_t*) msg, 22);
//
//		// Read a character from the FIFO.
//		receivedChar = SCI_readCharBlockingFIFO(SCIA_BASE);
//
//		// Echo back the character.
//		msg = "  You sent: \0";
//		SCI_writeCharArray(SCIA_BASE, (uint16_t*) msg, 13);
//		SCI_writeCharBlockingFIFO(SCIA_BASE, receivedChar);
//
//		// Increment the loop count variable.
//		loopCounter++;
//	}
}
void SCI_writeCharArrayLF(uint32_t base, char *array,
                          uint16_t length) {
	array[length++] = '\n';
	array[length] = '\0';
	SCI_writeCharArray(base, (uint16_t *) array, length+1);
}

void MD_puts(const char *str, EOL_t EOL) {
	while (*str != '\0')
		SCI_writeCharBlockingNonFIFO(SCIA_BASE, (uint16_t) *str++);
	if (EOL == EOL_LF)
		SCI_writeCharBlockingFIFO(SCIA_BASE, (uint16_t) '\n');
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
	SCI_setConfig(SCIA_BASE, DEVICE_LSPCLK_FREQ, 115200, SCI_CONFIG_WLEN_8 | \
	              SCI_CONFIG_STOP_ONE | SCI_CONFIG_PAR_NONE);
	SCI_resetChannels(SCIA_BASE);
	SCI_resetRxFIFO(SCIA_BASE);
	SCI_resetTxFIFO(SCIA_BASE);
	SCI_clearInterruptStatus(SCIA_BASE, SCI_INT_TXFF | SCI_INT_RXFF);
	SCI_enableFIFO(SCIA_BASE);
	SCI_enableModule(SCIA_BASE);
	SCI_performSoftwareReset(SCIA_BASE);
}
/*!
Initialization sequence for EPWM modules.
*/
void MD_InitEpwm(const uint32_t sampleFreq) {
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
	const uint16_t SAMP = (uint16_t) SysCtl_getClock(DEVICE_OSCSRC_FREQ)/sampleFreq;
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

}
/*!
Initialization sequence for ADC modules.
*/
void MD_InitAdcs(void) {
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
void MD_InitDclExample_dl(const uint32_t sampleFreq) {

	MD_InitEpwm(sampleFreq);
	MD_InitAdcs();
	/* enable interrupts */
	Interrupt_register(INT_ADCA1, &control_Isr);
	Interrupt_enable(INT_ADCA1);
	Interrupt_enableInCPU(M_INT1);

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
	lk = 1.0f;		// control loop not saturated

	MD_EPWM_UP_START();
}
/*!
Initialization sequence from DCL example (Bitfield programming model).
*/
#ifdef BF_MODEL
void MD_InitDclExample_bf(void) {
	EALLOW;
	PieVectTable.ADCA1_INT = &control_Isr;
	/* configure ePWM1 */
	CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
	EDIS;
	//	InitEPwm();								// [F2806x_EPwm.c]
	EPwm1Regs.TBCTL.bit.CTRMODE = 3;		// freeze TB counter
	EPwm1Regs.TBCTL.bit.PRDLD = 1;  		// immediate load
	EPwm1Regs.TBCTL.bit.PHSEN = 0;	   		// disable phase loading
	EPwm1Regs.TBCTL.bit.SYNCOSEL = 3;		// disable SYNCOUT signal
	EPwm1Regs.TBCTL.bit.HSPCLKDIV = 0;		// TBCLK = SYSCLKOUT
	EPwm1Regs.TBCTL.bit.CLKDIV = 0;			// clock divider = /1
	EPwm1Regs.TBCTL.bit.FREE_SOFT = 2;		// free run on emulation suspend
	EPwm1Regs.TBPRD = 0x2328;	        // set period for ePWM1 (0x2328 = 10kHz)
	EPwm1Regs.TBPHS.all = 0;			    // time-base Phase Register
	EPwm1Regs.TBCTR = 0;					// time-base Counter Register
	EPwm1Regs.ETSEL.bit.SOCAEN = 1;        	// enable SOC on A group
	EPwm1Regs.ETSEL.bit.SOCASEL = 1;       	// select SOC from zero match
	EPwm1Regs.ETPS.bit.SOCAPRD = 1;        	// generate pulse on 1st event
	EPwm1Regs.CMPCTL.bit.SHDWAMODE = 0;		// enable shadow mode
	EPwm1Regs.CMPCTL.bit.LOADAMODE = 2; 	// reload on CTR=zero
	//	EPwm1Regs.CMPA.half.CMPA = 0x0080;	 	// set compare A value
	EPwm1Regs.CMPA.all = 0x0080;			// set compare A value

	EPwm1Regs.AQCTLA.bit.CAU = AQ_SET;		// HIGH on CMPA up match
	EPwm1Regs.AQCTLA.bit.ZRO = AQ_CLEAR;	// LOW on zero match
	EALLOW;
	CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
	EDIS;

	/* configure ADC */
	//	InitaAdc();								// [F2806x_Adc.c]
	EALLOW;
	AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 0;		// early interrupt generation
	AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;		// enabled ADCINT1
	AdcaRegs.ADCINTSEL1N2.bit.INT1CONT = 0;	 	//  DCINT1 continuous mode
	AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 1;		// setup EOC1 to trigger ADCINT1
	AdcaRegs.ADCINTSEL1N2.bit.INT2E = 0;	    // enable ADCINT2
	AdcaRegs.ADCINTSEL1N2.bit.INT2CONT = 0;	  	// disable ADCINT1 continuous mode
	AdcaRegs.ADCINTSEL1N2.bit.INT2SEL = 0;	    // setup EOC1 to trigger ADCINT2
	AdcaRegs.ADCSOC0CTL.bit.CHSEL = 0;	// set SOC0 channel select to ADCINA0
	AdcaRegs.ADCSOC1CTL.bit.CHSEL = 8;	// set SOC1 channel select to ADCINB0
	AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 5;// set SOC0 start trigger on EPWM1A, due to round-robin SOC0 converts first then SOC1
	AdcaRegs.ADCSOC1CTL.bit.TRIGSEL = 5;// set SOC1 start trigger on EPWM1A, due to round-robin SOC0 converts first then SOC1
	AdcaRegs.ADCSOC0CTL.bit.ACQPS = 6;	// set SOC0 S/H Window to 7 ADC Clock Cycles, (6 ACQPS plus 1)
	AdcaRegs.ADCSOC1CTL.bit.ACQPS = 6;	// set SOC1 S/H Window to 7 ADC Clock Cycles, (6 ACQPS plus 1)
	EDIS;

	/* configure GPIO */
	//	InitGpio();  							// [F2806x_Gpio.c]
	EALLOW;
	GpioCtrlRegs.GPBMUX1.bit.GPIO34 = 0;	// GPIO34 = I/O pin
	GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1;		// GPIO34 = output
	GpioDataRegs.GPBSET.bit.GPIO34 = 1;		// GPIO34 = 1
	GpioCtrlRegs.GPBMUX1.bit.GPIO39 = 0;	// GPIO39 = I/O pin
	GpioCtrlRegs.GPBDIR.bit.GPIO39 = 1;		// GPIO39 = output
	GpioDataRegs.GPBCLEAR.bit.GPIO39 = 1;	// GPIO39 = 0
	EDIS;

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

	rk = 0.25f;							// initial value for control reference
	lk = 1.0f;								// control loop not saturated

	/* enable interrupts */
	PieCtrlRegs.PIEIER1.bit.INTx1 = 1; 	// enable PIE INT 1.1 (ADCINT1) - [adcisr]
	IER |= M_INT1;				// enable core interrupt 1 (ADC) - [control_isr]
	//	SetDBGIER(0x0001);						// enable real-time debug interupts
	EINT;
	// enable global interrupt mask

	EALLOW;
	EPwm1Regs.TBCTL.bit.CTRMODE = 0;		// PWM1 timer: count up and start
	EDIS;
}
#endif

/**
 * @brief      Simple digit to char converter.
 *
 * @param      number   The number
 * @param[in]  base     The base
 * @return     character representation of number
 */
char makedigit (uint32_t *number, uint32_t base) {
	static char map[] = "0123456789";
	uint16_t ix;

	for (ix=0; *number >= base; ix++)
		*number -= base;

	return map[ix];
}

/**
 * @brief      Simple integral to string converter.
 *
 * @param[in]  number   The number
 * @return     Pointer to string representation of number.
 */
char *int2str (const uint32_t number, char *buff) {
//	static char tmp[12] = { " " };
//	char *p = &tmp[0];
	uint32_t _number = number;
	bool firstNonzero = false;

	if (! _number) {
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
	Duty = (uk / 2.0f + 0.5f) * (float) EPwm1Regs.TBPRD;
	EPwm1Regs.CMPA.all = (Uint16) Duty;

	IsrCount++;
}
