/**
 * @file        adcEpwmSoc_dclClaPid.c
 * @project		06_adcEpwmSoc_dclClaPid
 *
 * @date        May 31, 2017
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
#include <string.h>						//!< Standard includes
/* ------------------------------------------------------------------------- */
#include "driverlib.h"
#include "device.h"						//!< Driver library includes
/* ------------------------------------------------------------------------- */
#include "F28x_Project.h"
#include "F2837xD_GlobalPrototypes.h"	//!< DSP bit field
/* ------------------------------------------------------------------------- */
#include "DCL.h"						//!< Digital controller library
/* ------------------------------------------------------------------------- */
#include "F2837xD_Cla_defines.h"		//!< Control law accelerator
/* ------------------------------------------------------------------------- */
#include "adcEpwmSoc_dclClaPid.h"		//!< Application
#include "md_globals.h"
#include "md_config.h"
#include "md_helpers.h"
#include "md_epwm.h"
#include "md_sci.h"
/* ------------------------------------------------------------------------- */
#include "cla_shared.h"

#define   Ts	 				25e-6
#define   BAUDRATE				230400U
#define	  F_CPU					200000000U

#define   LED_RD(x)				GPIO_writePin(31, x)
#define   LED_BL(x)				GPIO_writePin(34, x)
#define   IO_SET_DBG(PIN, x)	GPIO_writePin(PIN, x)
#define   IO_TGL_DBG(PIN)		GPIO_togglePin(PIN)

#define   FILTER_LEN			5
#define   ADC_BUF_LEN			256
#define   SCI_RX_BUFFER_SIZE 	16
#define   WAITSTEP              asm(" RPT #255 || NOP")

PID_t pid1 =
	{ .param = PID_DEFAULTS, .rk = 0.0f, .mk = 0.0f, .lk = 0.0f, .uk = 0.0f };
uint16_t adcaRes[ADC_BUF_LEN];
uint16_t SampleCount;
uint16_t resultsIndex;

//!< Task 1 (ASM) Variables
#ifdef __cplusplus
#pragma DATA_SECTION("Cla1ToCpuMsgRAM")
uint16_t voltFilt;
#pragma DATA_SECTION("Cla1ToCpuMsgRAM")
float X[FILTER_LEN];
#pragma DATA_SECTION(A,"CpuToCla1MsgRAM");
#if LOWPASS
float A[FILTER_LEN] = {0.0625L, 0.25L, 0.375L, 0.25L, 0.0625L};
#elif HIGHPASS
float A[FILTER_LEN] = {0.0625L, -0.25L, 0.375L, -0.25L, 0.0625L};
#endif
#else
#pragma DATA_SECTION(voltFilt,"Cla1ToCpuMsgRAM")
uint16_t voltFilt;
#pragma DATA_SECTION(X,"Cla1ToCpuMsgRAM")
float X[FILTER_LEN];
#pragma DATA_SECTION(A,"CpuToCla1MsgRAM");
float A[FILTER_LEN];
#endif

void CLA_configClaMemory(void);
void CLA_initCpu1Cla1(void);

__interrupt void IRQ_EPWM1(void);
__interrupt void IRQ_SCIA_rxFIFO(void);
__interrupt void IRQ_SCIA_txFIFO(void);
#ifdef ADC_HANDLER
#define   RESULTS_BUFFER_SIZE 	256
__interrupt void IRQ_ADCA_1(void);
#endif

__interrupt void cla1Isr1();
__interrupt void cla1Isr2();
__interrupt void cla1Isr3();
__interrupt void cla1Isr4();
__interrupt void cla1Isr5();
__interrupt void cla1Isr6();
__interrupt void cla1Isr7();
__interrupt void cla1Isr8();

static struct iface {
	char sciRxBuff[SCI_RX_BUFFER_SIZE], *pRxBuff, *rxCmdQue[10];
	short rxNewCmdsCtr;
	bool cmdRxBuffFull, cmdQueFull;
} i =
	{ .pRxBuff = &i.sciRxBuff[0], .cmdRxBuffFull = false, .cmdQueFull = false,
	        .rxNewCmdsCtr = 0, };

/*
 * ===================== main of 06_adcEpwmSoc_dclClaPid =====================
 */
void main(void) {
	Device_init();				//!< Initialize device clock and peripherals

//	SysCtl_disableWatchdog();
//	memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (size_t)&RamfuncsLoadSize);
//	Flash_initModule(FLASH0CTRL_BASE, FLASH0ECC_BASE, 3);
//	SysCtl_setClock(DEVICE_SETCLOCK_CFG);
//	SysCtl_setLowSpeedClock(SYSCTL_LSPCLK_PRESCALE_4);
//	ASSERT(SysCtl_getClock(DEVICE_OSCSRC_FREQ) == DEVICE_SYSCLK_FREQ);
//	ASSERT(SysCtl_getLowSpeedClock(DEVICE_OSCSRC_FREQ) == DEVICE_LSPCLK_FREQ);
//	Device_enableAllPeripherals();
//

	Device_initGPIO();			//!< Disable pin locks and enable internal pullups.
	//!< Initialize PIE, clear PIE registers, disable CPU interrupts
	Interrupt_initModule();
	//!< Initialize the PIE vector table with pointers to the shell ISR'S
	Interrupt_initVectorTable();
	//!< Setup all predefined GPIOs from
	MD_GPIO_Setup(&MD_GPIO_config[0], GPIO_COUNT_IN_PACKED_INIT_STRUCT);
	//!< Configure SCIA for UART operation
	MD_SCIx_init(SCIA_BASE, BAUDRATE);

	//!< Register interrupt callback handlers
	Interrupt_register(INT_EPWM1, &IRQ_EPWM1);
	Interrupt_register(INT_SCIA_RX, &IRQ_SCIA_rxFIFO);
#ifdef ADC_HANDLER
	Interrupt_register(INT_ADCA1, &IRQ_ADCA_1);
#endif
	// Initialize the PIE vector table with pointers to the shell Interrupt
	// Service Routines (ISR).
	// This will populate the entire table, even if the interrupt
	// is not used in this example.  This is useful for debug purposes.
	// The shell ISR routines are found in F2837xD_DefaultIsr.c.
	// This function is found in F2837xD_PieVect.c.
//	InitPieVectTable();

	// Step 4. Configure the CLA memory spaces first followed by
	// the CLA task vectors. Force task 8.
	CLA_configClaMemory();
	CLA_initCpu1Cla1();

	// Force CLA task 8 using the IACK instruction
	// Task 8 will initialize the filter input delay
	// line to zero (X[0] - X[4]).
	// No need to wait, the task will finish by the time
	// we configure the ePWM and ADC modules
	Cla1ForceTask8();

	//!< Set up the ADC and the ePWM and initialize the SOC
	MD_ADC_init(ADCA_BASE, ADC_RESOLUTION_12BIT, ADC_CLK_DIV_4_0);
	MD_EPWM_init(EPWM1_BASE, Ts, EPWM_SOC_A, EPWM_COUNTER_COMPARE_A, 1);
	MD_ADC_SOC_config(ADCA_BASE, ADC_CH_ADCIN3, ADC_INT_NUMBER1, ADC_SOC_NUMBER0, 75);
	MD_DAC_init(DACA_BASE, 2095);

	Interrupt_enable(INT_ADCA1);	//!< Enable ADC interrupt
	Interrupt_enable(INT_EPWM1);	//!< Enable EPWM interrupt
	Interrupt_enable(INT_SCIA_RX);	//!< Enable EPWM interrupt

	//!< Enable Global Interrupt (INTM) and real time interrupt (DBGM)
	EINT;
	ERTM;

	//!< Start ePWM1, enable SOCx and putting counter in run mode
	EPWM_enableADCTrigger(EPWM1_BASE, EPWM_SOC_A);
	EPWM_setTimeBaseCounterMode(EPWM1_BASE, EPWM_COUNTER_MODE_UP_DOWN);

	while (1) {
		_delay_ms(125);
		cmdIfaceHandler();
		MD_printi("ADCA1: ", adcaRes[ADC_SOC_NUMBER0], " \n");

		SCI_readCharBlockingFIFO(SCIA_BASE);

	}
}
/*
 * ===================== end of main of 06_adcEpwmSoc_dclClaPid ===============
 */

/**
 * CLA_initCpu1Cla1 - Initialize CLA1 task vectors and end of task interrupts
 */
void CLA_initCpu1Cla1(void) {
	// Compute all CLA task vectors
	// On Type-1 CLAs the MVECT registers accept full 16-bit task addresses as
	// opposed to offsets used on older Type-0 CLAs
	EALLOW;
	Cla1Regs.MVECT1 = (uint16_t) (&CLA1_PID1_calc);
	Cla1Regs.MVECT2 = (uint16_t) (&Cla1Task2);
	Cla1Regs.MVECT3 = (uint16_t) (&Cla1Task3);
	Cla1Regs.MVECT4 = (uint16_t) (&Cla1Task4);
	Cla1Regs.MVECT5 = (uint16_t) (&Cla1Task5);
	Cla1Regs.MVECT6 = (uint16_t) (&Cla1Task6);
	Cla1Regs.MVECT7 = (uint16_t) (&Cla1Task7);
	Cla1Regs.MVECT8 = (uint16_t) (&Cla1Task8);

	// Enable the IACK instruction to start a task on CLA in software
	// for all  8 CLA tasks. Also, globally enable all 8 tasks (or a
	// subset of tasks) by writing to their respective bits in the
	// MIER register
	Cla1Regs.MCTL.bit.IACKE = 1;
	Cla1Regs.MIER.all = (M_INT8 | M_INT7);

	// Configure the vectors for the end-of-task interrupt for all
	// 8 tasks
	PieVectTable.CLA1_1_INT = &cla1Isr1;
	PieVectTable.CLA1_2_INT = &cla1Isr2;
	PieVectTable.CLA1_3_INT = &cla1Isr3;
	PieVectTable.CLA1_4_INT = &cla1Isr4;
	PieVectTable.CLA1_5_INT = &cla1Isr5;
	PieVectTable.CLA1_6_INT = &cla1Isr6;
	PieVectTable.CLA1_7_INT = &cla1Isr7;
	PieVectTable.CLA1_8_INT = &cla1Isr8;

	// Set the adca.1 as the trigger for task 7
	DmaClaSrcSelRegs.CLA1TASKSRCSEL2.bit.TASK7 = 1;

	// Enable CLA interrupts at the group and subgroup levels
	PieCtrlRegs.PIEIER11.all = 0xFFFF;
	IER |= (M_INT11);
	EDIS;
}

/**
 * CLA_configClaMemory - Configure CLA memory sections
 */
void CLA_configClaMemory(void) {
	extern uint32_t Cla1funcsRunStart, Cla1funcsLoadStart, Cla1funcsLoadSize;

	EALLOW;
#ifdef _FLASH
	// Copy over code from FLASH to RAM
	memcpy((uint32_t *) &Cla1funcsRunStart, (uint32_t *) &Cla1funcsLoadStart,
	        (uint32_t) &Cla1funcsLoadSize);
#endif //_FLASH

	// Initialize and wait for CLA1ToCPUMsgRAM
	MemCfgRegs.MSGxINIT.bit.INIT_CLA1TOCPU = 1;
	while (MemCfgRegs.MSGxINITDONE.bit.INITDONE_CLA1TOCPU != 1) {
	};

	// Initialize and wait for CPUToCLA1MsgRAM
	MemCfgRegs.MSGxINIT.bit.INIT_CPUTOCLA1 = 1;
	while (MemCfgRegs.MSGxINITDONE.bit.INITDONE_CPUTOCLA1 != 1) {
	};

	// Select LS4RAM and LS5RAM to be the programming space for the CLA
	// First configure the CLA to be the master for LS4 and LS5 and then
	// set the space to be a program block
	MemCfgRegs.LSxMSEL.bit.MSEL_LS4 = 1;
	MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS4 = 1;
	MemCfgRegs.LSxMSEL.bit.MSEL_LS5 = 1;
	MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS5 = 1;

	EDIS;
}

__interrupt void cla1Isr1(void) {
	asm(" ESTOP0");
}		//!< cla1Isr1 - CLA1 ISR 1
__interrupt void cla1Isr2(void) {
	asm(" ESTOP0");
}		//!< cla1Isr2 - CLA1 ISR 2
__interrupt void cla1Isr3(void) {
	asm(" ESTOP0");
}		//!< cla1Isr3 - CLA1 ISR 3
__interrupt void cla1Isr4(void) {
	asm(" ESTOP0");
}		//!< cla1Isr4 - CLA1 ISR 4
__interrupt void cla1Isr5(void) {
	asm(" ESTOP0");
}		//!< cla1Isr5 - CLA1 ISR 5
__interrupt void cla1Isr6(void) {
	asm(" ESTOP0");
}		//!< cla1Isr6 - CLA1 ISR 6

/**
 * Post process after PID calculation.
 */
__interrupt void cla1Isr7(void) {
	//!< Clear the ADC interrupt flag so the next SOC can occur
	//!< Clear the PIEACK bits so another interrupt can be taken
	AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;  //make sure INT1 flag is cleared
	PieCtrlRegs.PIEACK.all = (PIEACK_GROUP1 | PIEACK_GROUP11);

	//!< Read the raw ADC RESULT1 register value and put it into the AdcBuf buffer
	// This can be compared to the CLA filtered value
//	adcaRes[SampleCount] = AdcaResultRegs.ADCRESULT0;

	// Read the CLA calculated value and put it in the
	// AdcFiltBuf buffer
//	adcFilRes[SampleCount++] = voltFilt;

	// Make sure that the buffer does not overflow
	// the buffer size.  If it is larger than ADC_BUF_LEN
	// then rewind back to the sample 0
//	if (SampleCount == ADC_BUF_LEN) {
//		SampleCount = 0;
	//    asm(" ESTOP0");
//	}

	// Uncomment to halt debugger here
}

__interrupt void cla1Isr8(void) {						//!< cla1Isr8 - CLA1 ISR 8
	// Acknowledge the end-of-task interrupt for task 8
	PieCtrlRegs.PIEACK.all = M_INT11;

	// Uncomment to halt debugger here
	//    asm(" ESTOP0");
}

void cmdIfaceHandler(void) {
	if (*(i.pRxBuff - 1) == '\n') {
		if (i.cmdQueFull) {
			MD_puts("Warn: Cmd flushed, Que full", EOL_LF);
		}
		else {
			strcpy(i.pRxBuff, i.rxCmdQue[i.rxNewCmdsCtr++]);
		}
	}
	else {
		if (i.cmdRxBuffFull) {
			MD_puts("Warn: long cmd flushed", EOL_LF);
			i.cmdRxBuffFull = false;
		}
		else {
			return;
		}

	}

	i.pRxBuff = &i.sciRxBuff[0];
	if (i.rxNewCmdsCtr > 0) {
		if (cmdIfaceExec(i.rxCmdQue[i.rxNewCmdsCtr--]) != 0) {
			MD_puts("Warn: cmdIfaceExec returns non-zero", EOL_LF);
		}
	}
	else {
		return;
	}
}

int cmdIfaceExec(const char *cmd) {
	MD_puts(cmd, EOL_LF);
	return 0;
}
/**
 * SCIA receive FIFO IRQ handler
 */
__interrupt void IRQ_SCIA_rxFIFO(void) {
	const char rxC = (char) SCI_readCharNonBlocking(SCIA_BASE);

	if ((i.pRxBuff + 1) > &i.sciRxBuff[0] + SCI_RX_BUFFER_SIZE) HALT();

	*i.pRxBuff++ = rxC;

	if (i.pRxBuff == &i.sciRxBuff[0] + SCI_RX_BUFFER_SIZE) {
		i.cmdRxBuffFull = true;
		Interrupt_disable(INT_SCIA_RX);
	}

	SCI_clearOverflowStatus(SCIA_BASE);
	SCI_clearInterruptStatus(SCIA_BASE, SCI_INT_RXFF);

	// Issue PIE ack
	Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);
}

/**
 * SCIA transmit FIFO IRQ handler
 */
__interrupt void IRQ_SCIA_txFIFO(void) {

}

/**
 * IRQ callback handler for EPWM1 irqs
 */
__interrupt void IRQ_EPWM1(void) {
	//!< Clear INT flag for this timer
	EPWM_clearEventTriggerInterruptFlag(EPWM1_BASE);

	//!< Acknowledge interrupt group
	Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP3);
}

/**
 * Power up and configure ADC module, Configure SOCx of ADCx to convert pin CH_ADCINx.
 * The EPWM1SOCA signal will be the trigger.
 * @param ADCx_BASE
 * @param resolution
 * @param clk_div
 */
void MD_ADC_init(const uint32_t ADCx_BASE, ADC_Resolution resolution,
                 ADC_ClkPrescale clk_div) {
	/*
	 *  Power-Up Sequence
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

	//!< Set ADCx's input ADCCLK divider
	ADC_setPrescaler(ADCx_BASE, clk_div);

	//!< Set resolution and signal mode (see #defines above) and load
	//! corresponding trims.
	ADC_setMode(ADCx_BASE, resolution, ADC_MODE_SINGLE_ENDED);

	//!< Set pulse positions to late
	ADC_setInterruptPulseMode(ADCx_BASE, ADC_PULSE_END_OF_CONV);

	//!< Power up the ADC and wait
	ADC_enableConverter(ADCx_BASE);
	DEVICE_DELAY_US(1000);
}

/**
 * Configure ePWMx to generate the SOC_A/B events.
 * @param EPWMx_BASE
 * @param samplePeriode
 * @param SOC_AB
 * @param CounterCompareModule
 * @param preScaleCount
 */
void MD_EPWM_init(const uint32_t EPWMx_BASE, float samplePeriode,
                  EPWM_ADCStartOfConversionType SOC_AB,
                  EPWM_CounterCompareModule CounterCompareModule, uint16_t preScaleCount) {
	//!< Disable SOC_A/B
	EPWM_disableADCTrigger(EPWMx_BASE, SOC_AB);

	//!< Configure the SOC to occur on the first up-count event
	EPWM_setADCTriggerSource(EPWMx_BASE, SOC_AB,
	        (SOC_AB == EPWM_SOC_A) ? EPWM_SOC_TBCTR_U_CMPA : EPWM_SOC_TBCTR_U_CMPB);

	//!< Set the event count prescaler
	EPWM_setADCTriggerEventPrescale(EPWMx_BASE, SOC_AB, preScaleCount);

	//!< Set the compare A/B and sample period registers
	EPWM_setCounterCompareValue(EPWMx_BASE, CounterCompareModule,
	        _CMPA(1e6 * samplePeriode / 10));

	EPWM_setTimeBasePeriod(EPWMx_BASE, _TBPRD(1e6 * samplePeriode));

	/**
	 * TBC run mode: UP_DOWN
	 * - Ignore event TBCTR == CMPA when in up counting direction
	 * - Do noting when TBCTR == TBPRD
	 * - Force EPWMxA pin high when TBCTR == CMPA in down counting direction
	 * - Force EPWMxA pin low when TBCTR == 0!
	 */
	EALLOW;
	EPwm1Regs.AQCTLA.bit.CAD = ACT_FORCE_HIGH;	//!< Event Compare A while down counting
	EPwm1Regs.AQCTLA.bit.CAU = ACT_IGNORE;			//!< Event Compare A while up counting
	EPwm1Regs.AQCTLA.bit.PRD = ACT_IGNORE;			//!< Event counter equals period
	EPwm1Regs.AQCTLA.bit.ZRO = ACT_FORCE_LOW;		//!< Event counter equals zero
	EDIS;

	EPWM_enableInterrupt(EPWMx_BASE);
	//!< Freeze the counter
	EPWM_setTimeBaseCounterMode(EPWMx_BASE, EPWM_COUNTER_MODE_STOP_FREEZE);
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

/**
 * Configure ADCx's SOCx to be triggered by ePWM events.
 * @param ADCx_BASE
 * @param CH_ADCINx
 * @param INT_NUMBERx
 * @param SOCx
 * @param sampleWindow_ns
 */
void MD_ADC_SOC_config(const uint32_t ADCx_BASE, ADC_Channel CH_ADCINx,
                       ADC_IntNumber INT_NUMBERx, ADC_SOCNumber SOCx,
                       uint32_t sampleWindow_ns) {

	//!< Assert sampleWindow_ns is modulo 5 divide-able then:
	//! sampleWindow 	= sampleWindow_ns / 5ns
	//! 				= sampleWindow_ns * 200 MHz
	ASSERT((sampleWindow_ns % 5) == 0);

	//!< Configure SOCx of ADCx to convert pin Ax. The EPWM1SOCA signal will be
	//! the trigger.
	if (ADC_getResolution(ADCx_BASE) == ADC_RESOLUTION_12BIT) ADC_setupSOC(ADCx_BASE,
	        SOCx, ADC_TRIGGER_EPWM1_SOCA, CH_ADCINx, (uint32_t) sampleWindow_ns / 5);

	if (ADC_getResolution(ADCx_BASE) == ADC_RESOLUTION_16BIT) ADC_setupSOC(ADCx_BASE,
	        SOCx, ADC_TRIGGER_EPWM1_SOCA, CH_ADCINx,
	        (uint32_t) sampleWindow_ns * (float) 4 / 5);

	//!< Set SOC0 to set the interrupt 1 flag. Enable the interrupt and make
	//! sure its flag is cleared.
	ADC_setInterruptSource(ADCx_BASE, INT_NUMBERx, SOCx);
	ADC_enableInterrupt(ADCx_BASE, INT_NUMBERx);
	ADC_clearInterruptStatus(ADCx_BASE, INT_NUMBERx);
}

/**
 * Query configured ADCx converter resolution.
 * @param ADCx_BASE
 * @return
 */
ADC_Resolution ADC_getResolution(const uint32_t ADCx_BASE) {
	ASSERT(ADC_isBaseValid(ADCx_BASE));
	return (ADC_Resolution) (HWREGH(ADCx_BASE + ADC_O_CTL2) & ADC_CTL2_RESOLUTION);
}

/**
 * Query current ADCx signal mode.
 * @param ADCx_BASE
 * @return
 */
ADC_SignalMode ADC_getSignalMode(const uint32_t ADCx_BASE) {
	ASSERT(ADC_isBaseValid(ADCx_BASE));
	return (ADC_SignalMode) (HWREGH(ADCx_BASE + ADC_O_CTL2) & ADC_CTL2_SIGNALMODE);
}

#ifdef ADC_HANDLER
/**
 * ADC_A Interrupt 1 ISR
 */
__interrupt void IRQ_ADCA_1(void) {
	//!< Add the latest result to the buffer
	adcRes[ADC_SOC_NUMBER0] = AdcaResultRegs.ADCRESULT0;

	//!< Clear INT1 flag and acknowledge PIE group 1
	AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}
#endif
