/**
 * @file        adc_soc_epwm_cpu01.c
 * @project		_adc_soc_epwm_cpu01
 *
 * @date        5 May 2017
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
//!< adc_soc_epwm_cpu01
#include "F28x_Project.h"
#include "F2837xD_GlobalPrototypes.h"
#include "md_beast_uart.h"
#include "DCL.h"

//#include "F2837xD_adc.h"

volatile struct DAC_REGS* DAC_PTR[4] =
	{ 0x0, &DacaRegs, &DacbRegs, &DaccRegs };
Uint16 dacVal = 2048;
short dir = 1;

uint16_t DAC_VALS_DACVALS_M = 4095;
uint16_t DAC_VALS_DACVALS_S = 1;

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


void MD_ADC_config(void);
void MD_ADC_configSoc(void);
void MD_EPWM_config(uint32_t sampleCycles);
void MD_DAC_config(uint16_t dac_num);
void MD_PID_instInit(PID *pidInst);
void MD_SCI_config(uint32_t baudrate);

interrupt void ADC_EOC_Irq(void);
interrupt void ADC_B_EOC_Irq(void);
interrupt void ADC_C_EOC_Irq(void);

//
// Defines
//
#define RESULTS_BUFFER_SIZE 256

//!< ADC channel definition
#define AIN_CHA_y	2		//!< ADCINB2 plant_a output
#define AIN_CHA_r	2		//!< ADCINC2 plant_a set-point
//#define AIN_CHB_y	?		//!< ADCINx3 plant_a output
//#define AIN_CHB_r	?		//!< ADCINx4 plant_a setpoint

#define TS	4096


//
// Globals
//


//
// Globals
//
//Uint16 AdcaResults[RESULTS_BUFFER_SIZE];
//Uint16 resultsIndex;
//volatile Uint16 bufferFull;
long IdleLoopCount = 0;
long IsrCount = 0;

float uk_dac;

PID pid1 = PID_DEFAULTS;	//!< PID instance
float rk = 0.25f;			//!< reference
float yk;					//!< feedback
float lk;					//!< saturation
float uk;					//!< control out

const char *msg = "Try Me \n";

/*
 * ===================== main of _md_adc_soc_epwm_dcl =====================
 */
void main(void) {
	InitSysCtrl();
	InitGpio();
	DINT;
	//!< Disable CPU interrupts

	InitPieCtrl();

	// Disable CPU interrupts and clear all CPU interrupt flags:
	IER = 0x0000;
	IFR = 0x0000;

	InitPieVectTable();

	MD_ADC_config();
	MD_ADC_configSoc();
	MD_EPWM_config(TS);

//	MD_SCI_config(115200);
	scia_uart_init(115200);

	MD_DAC_config(DAC_NUM);
	MD_PID_instInit(&pid1);

	MD_SCI_puts(msg);

	rk = 0.25f;							// initial value for control reference
	lk = 1.0f;							// control loop not saturated

	EALLOW;
	CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;	//!< sync ePWM
	EDIS;

	EPwm1Regs.ETSEL.bit.SOCAEN = 1;  		//!< enable SOCA
	EPwm1Regs.TBCTL.bit.CTRMODE = 0;  	//!< unfreeze, and enter up count mode

	PieCtrlRegs.PIECTRL.bit.ENPIE = 1;   // Enable the PIE block
	PieCtrlRegs.PIEIER9.bit.INTx1 = 1;   // PIE Group 9, INT1
	PieCtrlRegs.PIEIER9.bit.INTx2 = 1;   // PIE Group 9, INT2
	IER = 0x100;                         // Enable CPU INT

	EINT;								//!< Enable Global interrupt INTM
	ERTM;								//!< Enable Global realtime interrupt DBGM

    //
    // Step 5. User specific code, enable interrupts:
    //
    // Init send data.  After each transmission this data
    // will be updated for the next transmission
    //

       /* idle loop */
	while (1) {
		MD_SCI_puts(msg);
//		SCI_write(-1, msg, sizeof(msg)/sizeof(*msg));
		IdleLoopCount++;
		asm(" NOP");
	}
}


/* control ISR: triggered by ADC EOC */
interrupt void ADC_EOC_Irq(void) {
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
	AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;

	// read ADC channel
	yk = ((float) AdcbResultRegs.ADCRESULT0 - 2048.0f) / 2047.0f;
	rk = ((float) AdccResultRegs.ADCRESULT0 - 2048.0f) / 2047.0f;
	lk = 1.0f;

	DCL_runClamp_C1(&lk, 1.0f, 0.0f);//!< external clamp for anti-windup reset
	uk = DCL_runPID_C4(&pid1, rk, yk, lk);				//!< run PID controller

	uk_dac = (uk / 2.0f + 0.5f) * (float) DAC_VALS_DACVALS_M;//!< write u(k) to PWM
//	DAC_PTR[DAC_NUM]->DACVALS.all = (rk / 2.0f + 0.5f) * (float) DAC_VALS_DACVALS_M;
	DAC_PTR[DAC_NUM]->DACVALS.all = uk_dac;

//	EPwm1Regs.CMPA.all  = (Uint16) Duty;
	IsrCount++;
}

void MD_EPWM_config(uint32_t sampleCycles) {
	EALLOW;
	EPwm1Regs.ETSEL.bit.SOCAEN = 0;    // Disable SOC on A group
	EPwm1Regs.ETSEL.bit.SOCASEL = 4;   // Select SOC on up-count
	EPwm1Regs.ETPS.bit.SOCAPRD = 1;       // Generate pulse on 1st event
	EPwm1Regs.CMPA.bit.CMPA = 0.5 * sampleCycles;  // Set compare A value to 2048 counts
	EPwm1Regs.TBPRD = sampleCycles;             // Set period to 4096 counts
	EPwm1Regs.TBCTL.bit.CTRMODE = 3;      // freeze counter
	EDIS;
}

void MD_ADC_configSoc(void) {
	EALLOW;
	AdcaRegs.ADCSOC0CTL.bit.CHSEL = 3;  //SOC0 will convert pin A0
	AdcaRegs.ADCSOC0CTL.bit.ACQPS = 19;  //sample window is 100 SYSCLK cycles
	AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 5;  //trigger on ePWM1 SOCA/C

	AdcbRegs.ADCSOC0CTL.bit.CHSEL = 2;  //SOC0 will convert pin A0
	AdcbRegs.ADCSOC0CTL.bit.ACQPS = 19;  //sample window is 100 SYSCLK cycles
	AdcbRegs.ADCSOC0CTL.bit.TRIGSEL = 5;  //trigger on ePWM1 SOCA/C

	AdccRegs.ADCSOC0CTL.bit.CHSEL = 2;  //SOC0 will convert pin A0
	AdccRegs.ADCSOC0CTL.bit.ACQPS = 19;  //sample window is 100 SYSCLK cycles
	AdccRegs.ADCSOC0CTL.bit.TRIGSEL = 5;  //trigger on ePWM1 SOCA/C

	AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 0;  //end of SOC0 will set INT1 flag
	AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;   //enable INT1 flag
	AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;  //make sure INT1 flag is cleared

	PieVectTable.ADCA1_INT = &ADC_EOC_Irq;	//!< function for ADCA interrupt 1
	EDIS;

	IER |= M_INT1;  		//!< Enable group 1 interrupts
	PieCtrlRegs.PIEIER1.bit.INTx1 = 1;		//!< Enable PIE interrupt
}

void MD_ADC_config(void) {
	EALLOW;
	AdcaRegs.ADCCTL2.bit.PRESCALE = 6;  //set ADCCLK divider to /4
	AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
	AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;
	AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;
	DELAY_US(1000);

	AdcbRegs.ADCCTL2.bit.PRESCALE = 6;  //set ADCCLK divider to /4
	AdcSetMode(ADC_ADCB, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
	AdcbRegs.ADCCTL1.bit.INTPULSEPOS = 1;
	AdcbRegs.ADCCTL1.bit.ADCPWDNZ = 1;
	DELAY_US(1000);

	AdccRegs.ADCCTL2.bit.PRESCALE = 6;  //set ADCCLK divider to /4
	AdcSetMode(ADC_ADCC, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
	AdccRegs.ADCCTL1.bit.INTPULSEPOS = 1;
	AdccRegs.ADCCTL1.bit.ADCPWDNZ = 1;
	DELAY_US(1000);
	EDIS;
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



