/**
 * @file        md_beast_uart.c
 * @project		_MD_adc_soc_pid_dcl_cla
 *
 * @date        6 May 2017
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
#include "md_beast_uart.h"
#include "md_circ_buffer.h"
#include "F28x_Project.h"
#include "F2837xD_GlobalPrototypes.h"

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~  Private typedefs  ~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~  Private macro definitions  ~~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~  Private variable declaration  ~~~~~~~~~~~~~~~~~~~~ */
//static rbd_t _rbd;
//static char _rbmem[8];

char txBuff[16];    // Send data for SCI-A
char rxBuff[16];    // Received data for SCI-A

static rbd_t _rbdRx;
static rbd_t _rbdTx;
static char _rbmemRx[8];
static char _rbmemTx[8];

/* ~~~~~~~~~~~~~~~~~~~~~  Private function prototypes  ~~~~~~~~~~~~~~~~~~~~~ */
void scia_fifo_config(const uint32_t baudrate);

/* ~~~~~~~~~~~~~~~~~~~  Private function implementations  ~~~~~~~~~~~~~~~~~~ */


interrupt void sciaTxFifoIsr(void) {
	volatile uint16_t err = 0;

	for (short i=0; i < SCI_HW_TX_FIFO_DEPTH; i++)
		err = ring_buffer_get(_rbdTx, (uint16_t *) &SciaRegs.SCITXBUF.all);

	SciaRegs.SCIFFTX.bit.TXFFINTCLR = 1;   // Clear SCI Interrupt flag
	PieCtrlRegs.PIEACK.bit.ACK9 = 1;       // Issue PIE ACK
}


interrupt void sciaRxFifoIsr(void) {
	volatile uint16_t err = 0;

	for (short i=0; i < SCI_HW_RX_FIFO_DEPTH; i++)
		err = ring_buffer_put(_rbdRx, (uint16_t *) &SciaRegs.SCIRXBUF.all);


	SciaRegs.SCIFFRX.bit.RXFFOVRCLR = 1;   // Clear Overflow flag
	SciaRegs.SCIFFRX.bit.RXFFINTCLR = 1;   // Clear Interrupt flag

	PieCtrlRegs.PIEACK.bit.ACK9 = 1;       // Issue PIE ack
}

void MD_SCI_puts(const char * str) {
	ring_buffer_put(_rbdTx, (char *) str);
}


void scia_uart_init(const uint32_t baudrate) {
	GPIO_SetupPinMux(42, GPIO_MUX_CPU1, 1);
	GPIO_SetupPinOptions(42, GPIO_INPUT, GPIO_PUSHPULL);
	GPIO_SetupPinMux(43, GPIO_MUX_CPU1, 1);
	GPIO_SetupPinOptions(43, GPIO_OUTPUT, GPIO_ASYNC);

	// Interrupts that are used in this example are re-mapped to
	// ISR functions found within this file.
	EALLOW;  // This is needed to write to EALLOW protected registers
	PieVectTable.SCIA_RX_INT = &sciaRxFifoIsr;
	PieVectTable.SCIA_TX_INT = &sciaTxFifoIsr;
	EDIS;    // This is needed to disable write to EALLOW protected registers

	scia_fifo_config(baudrate);

	/* Initialize the ring buffer */
	rb_attr_t attrRx = {
			sizeof(_rbmemRx[0]),
			sizeof(_rbmemRx) / sizeof(_rbmemRx[0]),
			_rbmemRx
	};

	rb_attr_t attrTx = {
			sizeof(_rbmemTx[0]),
			sizeof(_rbmemTx) / sizeof(_rbmemTx[0]),
			_rbmemTx
	};

	ring_buffer_init(&_rbdRx, &attrRx);
	ring_buffer_init(&_rbdTx, &attrTx);
}

void scia_fifo_config(const uint32_t baudrate) {
	SciaRegs.SCICCR.all = 0x0007;      // 1 stop bit,  No loopback
	// No parity,8 char bits,
	// async mode, idle-line protocol
	SciaRegs.SCICTL1.all = 0x0003;     // enable TX, RX, internal SCICLK,
	// Disable RX ERR, SLEEP, TXWAKE
	SciaRegs.SCICTL2.bit.TXINTENA = 1;
	SciaRegs.SCICTL2.bit.RXBKINTENA = 1;
	SciaRegs.SCIHBAUD.all = 0x0000;
	SciaRegs.SCILBAUD.all = LSPCLK_DEFAULT / (baudrate * 8) - 1;;
	SciaRegs.SCICCR.bit.LOOPBKENA = 0;  // Enable loop back
	SciaRegs.SCIFFTX.all = 0xC022;
	SciaRegs.SCIFFRX.all = 0x0022;
	SciaRegs.SCIFFCT.all = 0x00;

	SciaRegs.SCICTL1.all = 0x0023;     // Relinquish SCI from Reset
	SciaRegs.SCIFFTX.bit.TXFIFORESET = 1;
	SciaRegs.SCIFFRX.bit.RXFIFORESET = 1;
}

