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

#include <__MD_LIB/md_globals.h>
#include <__MD_LIB/md_gpio.h>
#include <__MD_LIB/md_sci.h>
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
//#include "DCL.h"
/* ------------------------------------------------------------------------- */
#include "md_config.h"
#include "md_epwm.h"

/*
 * ===================== main of _E_epwm_gpio_toggle =====================
 */
void main(void) {
	Device_init();				//!< Initialize device clock and peripherals
	Device_initGPIO();			//!< Disable pin locks and enable internal pullups.

#ifdef _FLASH
	//!< Initialize the FLASH
	memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (unsigned long)(&RamfuncsLoadSize));
	Flash_initModule(FLASH0CTRL_BASE, FLASH0ECC_BASE, DEVICE_FLASH_WAITSTATES);
#endif

	//!< Setup all predefined GPIOs from
	MD_GPIO_Setup(&MD_GPIO_config[0], GPIO_COUNT_IN_PACKED_INIT_STRUCT);
	//!< Initialize PIE, clear PIE registers, disable CPU interrupts
	//!< Initialize the PIE vector table with pointers to the shell ISR'S
	Interrupt_initModule();
	Interrupt_initVectorTable();

	//!< Register IRQ callback handlers.
	Interrupt_register(INT_EPWM1, &IRQ_EPWM_1);

	//!< Configure ePWM module 1 for ??us period (sample frequency 1/100us)
	MD_EPWM_init(EPWM1_BASE, 50);

	Interrupt_enable(INT_EPWM1);

	//!< Start ePWM1, enabling SOCA trigger and putting the counter in up-count mode
	EPWM_setTimeBaseCounterMode(EPWM1_BASE, EPWM_COUNTER_MODE_UP_DOWN);

	//!< Enable Global Interrupt (INTM) and realtime interrupt (DBGM).
	EINT;
	ERTM;

	while (1) {
//		MD_printi("pref: ", gc_slider, " post\n");
		DEVICE_DELAY_US(10);
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
