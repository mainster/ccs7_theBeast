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

__interrupt void IRQ_EPWM1(void);
__interrupt void IRQ_ADCA_1(void);
__interrupt void IRQ_SCIA_rxFIFO(void);
__interrupt void IRQ_SCIA_txFIFO(void);

void MD_ADC_init(const uint32_t ADCx_BASE, ADC_Resolution resolution, ADC_ClkPrescale clk_div);

void MD_EPWM_init(const uint32_t EPWMx_BASE, float samplePeriode,
                  EPWM_ADCStartOfConversionType SOC_AB,
                  EPWM_CounterCompareModule CounterCompareModule, uint16_t preScaleCount);

void MD_DAC_init(const uint32_t DACx_BASE, const uint16_t initialOut);

void MD_ADC_SOC_config(const uint32_t ADCx_BASE, ADC_Channel CH_ADCINx,
                       ADC_IntNumber INT_NUMBERx, ADC_SOCNumber SOCx,
                       uint32_t sampleWindow_ns);

ADC_Resolution ADC_getResolution(const uint32_t ADCx_BASE);
ADC_SignalMode ADC_getSignalMode(const uint32_t ADCx_BASE);

#define BAUDRATE			230400U
#define	F_CPU				200000000U

#define LED_RD(x)			GPIO_writePin(31, x)
#define LED_BL(x)			GPIO_writePin(34, x)
#define IO_SET_DBG(PIN, x)	GPIO_writePin(PIN, x)
#define IO_TGL_DBG(PIN)		GPIO_togglePin(PIN)

#define RESULTS_BUFFER_SIZE 	256
#define SCI_RX_BUFFER_SIZE 		16

Uint16 adcaRes[RESULTS_BUFFER_SIZE];
Uint16 resultsIndex;

static struct iface {
	char
	sciRxBuff[SCI_RX_BUFFER_SIZE],
	*pRxBuff;
	*rxCmdQue[10];
	short rxNewCmdsCtr;
	bool cmdRxed, cmdRxBuffFull, cmdQueFull;
} iface = {
		.pRxBuff 		= &iface.sciRxBuff[0],
		.cmdRxBuffFull	= false,
		.cmdRxed 		= false,
		.cmdQueFull 	= false,
		.rxNewCmdsCtr	= 0,
};


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

	//!< Register interrupt callback handlers
	Interrupt_register(INT_SCIA_RX, &IRQ_SCIA_rxFIFO);
	Interrupt_register(INT_ADCA1, 	&IRQ_ADCA_1);
	Interrupt_register(INT_EPWM1, 	&IRQ_EPWM1);

	//!< Set up the ADC and the ePWM and initialize the SOC
	MD_ADC_init(ADCA_BASE, ADC_RESOLUTION_12BIT, ADC_CLK_DIV_4_0);
	MD_EPWM_init(EPWM1_BASE, Ts, EPWM_SOC_A, EPWM_COUNTER_COMPARE_A, 1);
	MD_ADC_SOC_config(ADCA_BASE, ADC_CH_ADCIN3, ADC_INT_NUMBER1, ADC_SOC_NUMBER0, 75);

	Interrupt_enable(INT_ADCA1);	//!< Enable ADC interrupt
	Interrupt_enable(INT_EPWM1);	//!< Enable EPWM interrupt

	//!< Enable Global Interrupt (INTM) and real time interrupt (DBGM)
	EINT; ERTM;

	//!< Start ePWM1, enable SOCx and putting counter in run mode
	EPWM_enableADCTrigger(EPWM1_BASE, EPWM_SOC_A);
	EPWM_setTimeBaseCounterMode(EPWM1_BASE, EPWM_COUNTER_MODE_UP_DOWN);

	while (1) {
		_delay_ms(25);
		MD_printi("ADCA1: ", adcRes[ADC_SOC_NUMBER0], " \n");
	}
}
/*
 * ===================== end of main of 06_adcEpwmSoc_dclClaPid ===============
 */

void cmdIfaceHandler(void) {
	if (! iface.cmdRxBuffFull) {
		if (iface.cmdRxed) {
			iface.cmdRxed = false;
			iface.pRxBuff = &iface.sciRxBuff[0];

			if (iface.cmdQueFull) {
				// warn "Que full"
				return;
			}
			else {
				//copy
				// inc que ctr
			}

			Interrupt_enable(INT_SCIA_RX);
		}
		else {
			// warn "long_cmd"
			iface.pRxBuff = &iface.sciRxBuff[0];
		}
	}
	else {

	}

	if (rxC == '\n') {
		iface.pRxBuff = '\0';
	}
	else HALT();
	}
	else {
	if (rxC != '\n')
		iface.pRxBuff++ = rxC;
	else {
		iface.pRxBuff = '\0';
		iface.cmdRxed = true;
	}
}

/**
 * SCIA receive FIFO IRQ handler
 */
__interrupt void IRQ_SCIA_rxFIFO(void) {
    const char rxC = (char) SCI_readCharNonBlocking(SCIA_BASE);

	if ((iface.pRxBuff+1) > &iface.sciRxBuff[0] + SCI_RX_BUFFER_SIZE)
		HALT();

	*iface.pRxBuff++ = rxC;

	if (iface.pRxBuff == &iface.sciRxBuff[0] + SCI_RX_BUFFER_SIZE) {
    	iface.cmdRxBuffFull = true;
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
 * ADC_A Interrupt 1 ISR
 */
__interrupt void IRQ_ADCA_1(void) {
	//!< Add the latest result to the buffer
	adcRes[ADC_SOC_NUMBER0] = AdcaResultRegs.ADCRESULT0;

	//!< Clear INT1 flag and acknowledge PIE group 1
	AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
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
void MD_ADC_init(const uint32_t ADCx_BASE, ADC_Resolution resolution, ADC_ClkPrescale clk_div) {
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
                ADC_IntNumber INT_NUMBERx, ADC_SOCNumber SOCx, uint32_t sampleWindow_ns) {

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


