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
#include "md_helpers.h"
#include "md_epwm.h"
#include "md_sci.h"
#include "device/device.h"

volatile struct DAC_REGS* DAC_PTR[4] =
	{ 0x0, &DacaRegs, &DacbRegs, &DaccRegs };
Uint16 dacVal = 2048;
short dir = 1;

//uint16_t DAC_VALS_DACVALS_M = 4095;
//uint16_t DAC_VALS_DACVALS_S = 1;

//
// Defines
//
#define REFERENCE_VDAC      0
#define REFERENCE_VREF      1
#define DACA         1
#define DACB         2
#define DACC         3
#define REFERENCE            REFERENCE_VREF
#define DAC_NUM                DACA

// Function Prototypes
void ConfigureADC(void);
void ConfigureEPWM(void);
void initPIDvariables(void);
void SetupADCEpwm(Uint16 channel);
void SetupADCEpwmSync(Uint16 setpoint, Uint16 plant);
void configureDAC(Uint16 dac_num);

interrupt void ADC_EOC_Irq(void);
interrupt void ADC_B_EOC_Irq(void);
interrupt void ADC_C_EOC_Irq(void);

//
// Defines
//
#define RESULTS_BUFFER_SIZE 256

#define BAUDRATE			230400U
#define	F_CPU				200000000U

#define LED_RD(x)			GPIO_writePin(31, x)
#define LED_BL(x)			GPIO_writePin(34, x)
#define IO_SET_DBG(PIN, x)	GPIO_writePin(PIN, x)
#define IO_TGL_DBG(PIN)		GPIO_togglePin(PIN)


////!< ADC channel definition
//#define AIN_CHA_y	2		//!< ADCINB2 plant_a output
//#define AIN_CHA_r	2		//!< ADCINC2 plant_a set-point
////#define AIN_CHB_y	?		//!< ADCINx3 plant_a output
////#define AIN_CHB_r	?		//!< ADCINx4 plant_a setpoint

#define  Ts	 25e-6

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

/*
 * ===================== main of _md_adc_soc_epwm_dcl =====================
 */
void main(void) {
	InitSysCtrl();

	// Disable CPU interrupts and clear all CPU interrupt flags:
	IER = 0x0000;
	IFR = 0x0000;

	InitPieCtrl();
	InitPieVectTable();
	InitGpio();

	//!< Setup all predefined GPIOs from
	MD_GPIO_Setup(&MD_GPIO_config[0], GPIO_COUNT_IN_PACKED_INIT_STRUCT);
	//!< Configure SCIA for UART operation
	MD_SCIx_init(SCIA_BASE, BAUDRATE);

	EALLOW;
	PieVectTable.ADCA1_INT = &ADC_EOC_Irq;	//!< function for ADCA interrupt 1
	EDIS;

	EALLOW;
	AdcaRegs.ADCCTL2.bit.PRESCALE = 6;  //set ADCCLK divider to /4
	AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT_bf, ADC_SIGNALMODE_SINGLE);
	AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;
	AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;
	DELAY_US(1000);

	AdcaRegs.ADCSOC0CTL.bit.CHSEL = 3;  //SOC0 will convert pin A0
	AdcaRegs.ADCSOC0CTL.bit.ACQPS = 19;  //sample window is 100 SYSCLK cycles
	AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 5;  //trigger on ePWM1 SOCA/C

	AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 0;  //end of SOC0 will set INT1 flag
	AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;   //enable INT1 flag
	AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;  //make sure INT1 flag is cleared
	EDIS;

//	ConfigureEPWM();
	EALLOW;
	EPwm1Regs.ETSEL.bit.SOCAEN = 0;    // Disable SOC on A group
	EPwm1Regs.ETSEL.bit.SOCASEL = 4;   // Select SOC on up-count
	EPwm1Regs.ETPS.bit.SOCAPRD = 1;       // Generate pulse on 1st event
	EPwm1Regs.CMPA.bit.CMPA = _CMPA(Ts * 1e6);
	EPwm1Regs.TBPRD = _TBPRD(Ts * 1e6);
	EPwm1Regs.TBCTL.bit.CTRMODE = 3;      // freeze counter
	/**
	 * TBC run mode: UP_DOWN
	 * - Ignore event TBCTR == CMPA when in up counting direction
	 * - Do noting when TBCTR == TBPRD
	 * - Force EPWMxA pin high when TBCTR == CMPA in down counting direction
	 * - Force EPWMxA pin low when TBCTR == 0!
	 */
	EPwm1Regs.AQCTLA.bit.CAD = ACT_FORCE_HIGH;		//!< Event Compare A while down counting
	EPwm1Regs.AQCTLA.bit.CAU = ACT_IGNORE;			//!< Event Compare A while up counting
	EPwm1Regs.AQCTLA.bit.PRD = ACT_IGNORE;			//!< Event counter equals period
	EPwm1Regs.AQCTLA.bit.ZRO = ACT_FORCE_LOW;		//!< Event counter equals zero
	EDIS;


//	configureDAC(DAC_NUM);
	initPIDvariables();

	IER |= M_INT1;  		//!< Enable group 1 interrupts
	EINT;					//!< Enable Global interrupt INTM
	ERTM;					//!< Enable Global realtime interrupt DBGM

	PieCtrlRegs.PIEIER1.bit.INTx1 = 1;		//!< Enable PIE interrupt

	EALLOW;
	CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;	//!< sync ePWM
	EDIS;

	EPwm1Regs.ETSEL.bit.SOCAEN = 1;  		//!< enable SOCA
	EPwm1Regs.TBCTL.bit.CTRMODE = EPWM_COUNTER_MODE_UP_DOWN;

	/* idle loop */
	while (1) {
		IdleLoopCount++;
		asm(" NOP");
	}
}

// configureDAC - Configure specified DAC output
void configureDAC(Uint16 dac_num) {
	EALLOW;
	DAC_PTR[dac_num]->DACCTL.bit.DACREFSEL = REFERENCE;
	DAC_PTR[dac_num]->DACOUTEN.bit.DACOUTEN = 1;
	DAC_PTR[dac_num]->DACVALS.all = 0;
	DELAY_US(10);  // Delay for buffered DAC to power up
	EDIS;
}

void initPIDvariables(void) {
	/* Initialize controller variables */
	pid1.Kp = 3.0f;
	pid1.Ki = 0.2f;
	pid1.Kd = 0.0f;
	pid1.Kr = 1.0f;
	pid1.c1 = 1.0296600613396f;
	pid1.c2 = 0.880296600613396f;
	pid1.d2 = 0.0f;
	pid1.d3 = 0.0f;
	pid1.i10 = 0.0f;
	pid1.i14 = 1.0f;
	pid1.Umax = 1.0f;
	pid1.Umin = -1.0f;

	rk = 0.25f;							// initial value for control reference
	lk = 1.0f;								// control loop not saturated
}

//
// ConfigureADC - Write ADC configurations and power up the ADC for both
//                ADC A and ADC B
//
void ConfigureADC(void) {
	EALLOW;

	//write configurations
	AdcaRegs.ADCCTL2.bit.PRESCALE = 6;  //set ADCCLK divider to /4
//	AdcaRegs.ADCCTL2.bit.PRESCALE = 6;  //set ADCCLK divider to /4
	AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT_bf, ADC_SIGNALMODE_SINGLE);
//	AdcSetMode(ADC_ADCC, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);

	//Set pulse positions to late
	AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;
//	AdccRegs.ADCCTL1.bit.INTPULSEPOS = 1;

	//power up the ADC
	AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;
//	AdccRegs.ADCCTL1.bit.ADCPWDNZ = 1;

	//delay for 1ms to allow ADC time to power up
	DELAY_US(1000);

	EDIS;
}

//
// ConfigureEPWM - Configure EPWM SOC and compare values
//
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

void SetupADCEpwmSync(Uint16 setpoint, Uint16 plant) {
	//	ADCINB2: yk
	//	ADCINC2: rk
	EALLOW;
	AdcbRegs.ADCSOC0CTL.bit.CHSEL = setpoint; 	//SOC0 will convert ADCINxy
	AdcbRegs.ADCSOC0CTL.bit.ACQPS = 19; 		//SOC0 will use sample duration of 20 SYSCLK cycles
	AdcbRegs.ADCSOC0CTL.bit.TRIGSEL = 5; 		//SOC0 will begin conversion on ePWM3 SOCB
	AdccRegs.ADCSOC0CTL.bit.CHSEL = plant; 		//SOC0 will convert ADCINxy
	AdccRegs.ADCSOC0CTL.bit.ACQPS = 19; 		//SOC0 will use sample duration of 20 SYSCLK cycles
	AdccRegs.ADCSOC0CTL.bit.TRIGSEL = 5; 		//SOC0 will begin conversion on ePWM3 SOCB

	AdcbRegs.ADCINTSEL1N2.bit.INT1SEL = 0; 		//end of SOC0 will set INT1 flag
	AdcbRegs.ADCINTSEL1N2.bit.INT1E = 1;   		//enable INT1 flag
	AdcbRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;  	//make sure INT1 flag is cleared
	EDIS;
}

//
// SetupADCEpwm - Setup ADC EPWM acquisition window
//
void SetupADCEpwm(Uint16 channel) {
	Uint16 acqps;

	//determine minimum acquisition window (in SYSCLKS) based on resolution
	if (ADC_RESOLUTION_12BIT_bf == AdcaRegs.ADCCTL2.bit.RESOLUTION) {
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

/* control ISR: triggered by ADC EOC */
interrupt void ADC_EOC_Irq(void) {
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
	AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;

	// read ADC channel
	yk = ((float) AdcbResultRegs.ADCRESULT0 - 2048.0f) / 2047.0f;
	rk = ((float) AdccResultRegs.ADCRESULT0 - 2048.0f) / 2047.0f;
	lk = 1.0f;

	DCL_runClamp_C1(&lk, 1.0f, 0.0f);						//!< external clamp for anti-windup reset
	uk = DCL_runPID_C4(&pid1, rk, yk, lk);					//!< run PID controller

	uk_dac = (uk / 2.0f + 0.5f) * (float) DAC_VALS_DACVALS_M;	//!< write u(k) to PWM
//	DAC_PTR[DAC_NUM]->DACVALS.all = (rk / 2.0f + 0.5f) * (float) DAC_VALS_DACVALS_M;
	DAC_PTR[DAC_NUM]->DACVALS.all = uk_dac;

//	EPwm1Regs.CMPA.all  = (Uint16) Duty;
	IsrCount++;
}


/* control ISR: triggered by ADC EOC */
interrupt void ADC_B_EOC_Irq(void) {
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
	AdcbRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;
}

interrupt void ADC_C_EOC_Irq(void) {
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
	AdccRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;
}



