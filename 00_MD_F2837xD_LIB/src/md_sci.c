/**
 * @file        md_uart.c
 * @project		MD_F2837xD_LIB
 * 
 * @date        May 22, 2017
 * @author      Manuel Del Basso (mainster)
 * @email       manuel.delbasso@gmail.com
 *
 * @ide         Code Composer Studio Version: 7.1.0.00015 
 * @license		GNU GPL v3
 * 
 * @brief       TODO
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
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  Includes  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "driverlib.h"
#include "device.h"
#include "md_sci.h"
#include "md_helpers.h"
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~  Private typedefs  ~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~  Private macro definitions  ~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~  Private variable declaration  ~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~  Private function prototypes  ~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~  Private function implementations  ~~~~~~~~~~~~~~~~~~ */
//#define AUTOBAUD

/**
 * Initialization sequence for SCIx module.
 * @param SCIx_BASE address
 * @param baudRate
 */
void MD_SCIx_init(const uint32_t SCIx_BASE, const uint32_t baudRate) {
	//!< GPIO43 is the SCI Rx pin.
	GPIO_setMasterCore(43, GPIO_CORE_CPU1);
	GPIO_setPinConfig(GPIO_43_SCIRXDA);
	GPIO_setDirectionMode(43, GPIO_DIR_MODE_IN);
	GPIO_setPadConfig(43, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(43, GPIO_QUAL_ASYNC);

	//!< GPIO42 is the SCI Tx pin.
	GPIO_setMasterCore(42, GPIO_CORE_CPU1);
	GPIO_setPinConfig(GPIO_42_SCITXDA);
	GPIO_setDirectionMode(42, GPIO_DIR_MODE_OUT);
	GPIO_setPadConfig(42, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(42, GPIO_QUAL_ASYNC);

	//!< Initialize SCIx and its FIFO.
	SCI_performSoftwareReset(SCIx_BASE);

	//!< Configure communication interface
	SCI_setConfig(SCIx_BASE, DEVICE_LSPCLK_FREQ, baudRate,
	              SCI_CONFIG_WLEN_8 | SCI_CONFIG_STOP_ONE | SCI_CONFIG_PAR_NONE);

	SCI_resetChannels(SCIx_BASE);
	SCI_resetRxFIFO(SCIx_BASE);
	SCI_resetTxFIFO(SCIx_BASE);
	SCI_clearInterruptStatus(SCIx_BASE, SCI_INT_TXFF | SCI_INT_RXFF);
	SCI_enableFIFO(SCIx_BASE);
	SCI_enableModule(SCIx_BASE);
	SCI_performSoftwareReset(SCIx_BASE);

#ifdef AUTOBAUD
	// Perform an autobaud lock.
	// SCI expects an 'a' or 'A' to lock the baud rate.
	SCI_lockAutobaud(SCIx_BASE);
#endif

}

void MD_puts(const char *str, EOL_t EOL) {
	while (*str != '\0')
		SCI_writeCharBlockingNonFIFO(SCIA_BASE, (uint16_t) *str++);
	if (EOL == EOL_LF) SCI_writeCharBlockingFIFO(SCIA_BASE, (uint16_t) '\n');
}

void MD_putsJson(const char *key, uint16_t *buff, uint16_t size) {
	char i2sa[10];
	char *i2s = &i2sa[0];

	while (size--) {
		int2str(*buff++, i2s);
		MD_putKeyVal(key, i2s);
	}
}

void MD_printi(const char *prefix, uint32_t arg, const char *posfix) {
	char i2sa[10];
	char *i2s = &i2sa[0];

	int2str(arg, i2s);
	MD_puts(prefix, EOL_NO);
	MD_puts(i2s, EOL_NO);
	MD_puts(posfix, EOL_NO);
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
