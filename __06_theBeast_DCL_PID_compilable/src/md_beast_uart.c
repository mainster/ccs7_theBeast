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
#include "F28x_Project.h"
#include "F2837xD_GlobalPrototypes.h"
//#include "driverlib.h"
//#include "device.h"

#include "md_beast_uart.h"
#include "md_globals.h"
#include "md_circ_buffer.h"

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~  Private typedefs  ~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~  Private macro definitions  ~~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~  Private variable declaration  ~~~~~~~~~~~~~~~~~~~~ */
//static rbd_t _rbd;
//static char _rbmem[8];
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
static int rx_buff[RX0_SIZE];
static int * volatile rx_inp;
static int *rx_outp;
static int tx_buff[TX0_SIZE];
static int tx_in;
static int tx_out;

#define ROLLOVER( x, max )	x = ++x >= max ? 0 : x
#define ROLLOVERP( x, start, size )	x = ++x >= (start+size) ? start : x
// count up and wrap around

void init_uart0(const uint32_t baudrate) {
	//!< GPIO43 is the SCI Rx pin.
//	GPIO_setMasterCore(43, GPIO_CORE_CPU1);
//	GPIO_setPinConfig(GPIO_43_SCIRXDA);
//	GPIO_setDirectionMode(43, GPIO_DIR_MODE_IN);
//	GPIO_setPadConfig(43, GPIO_PIN_TYPE_STD);
//	GPIO_setQualificationMode(43, GPIO_QUAL_ASYNC);
//
//	//!< GPIO42 is the SCI Tx pin.
//	GPIO_setMasterCore(42, GPIO_CORE_CPU1);
//	GPIO_setPinConfig(GPIO_42_SCITXDA);
//	GPIO_setDirectionMode(42, GPIO_DIR_MODE_OUT);
//	GPIO_setPadConfig(42, GPIO_PIN_TYPE_STD);
//	GPIO_setQualificationMode(42, GPIO_QUAL_ASYNC);





	// Interrupts that are used in this example are re-mapped to
	// ISR functions found within this file.
	EALLOW;
	// This is needed to write to EALLOW protected registers
	PieVectTable.SCIA_RX_INT = &USART0_RX_vect;
	PieVectTable.SCIA_TX_INT = &USART0_UDRE_vect;
	EDIS;
	// This is needed to disable write to EALLOW protected registers

	scia_fifo_config(baudrate, 1, 1);

	//	UCSR0B = 1 << RXEN0 ^ 1 << TXEN0 ^		// enable RX, TX
//	        1 << RXCIE0;			// enable RX interrupt
	rx_inp = rx_buff;
	rx_outp = rx_buff;
	tx_in = tx_out;
}

int ukbhit0(void) {
	return rx_outp != rx_inp;
	// rx_in modified by interrupt !
}

int ugetchar0(void) {
	int data;

	while (!ukbhit0());;			// until at least one byte in
	data = *rx_outp;			// get byte
	ROLLOVERP(rx_outp, rx_buff, RX0_SIZE);
//	URX0_IEN = 1;				// enable RX interrupt
	SciaRegs.SCICTL2.bit.RXBKINTENA = 1;
	return data;
}

interrupt void USART0_RX_vect(void) {
	int* i = rx_inp;
	int *j = i;

	ROLLOVERP(i, rx_buff, RX0_SIZE);
	if (i == rx_outp) {			// buffer overflow
		SciaRegs.SCICTL2.bit.RXBKINTENA = 0;			// disable RX interrupt
		return;
	}
	*j = SciaRegs.SCIRXBUF.all;
	rx_inp = i;

	SciaRegs.SCIFFRX.bit.RXFFOVRCLR = 1;   // Clear Overflow flag
	SciaRegs.SCIFFRX.bit.RXFFINTCLR = 1;   // Clear Interrupt flag
	PieCtrlRegs.PIEACK.bit.ACK9 = 1;       // Issue PIE ack
}

interrupt void USART0_UDRE_vect(void) {
	if (tx_in == tx_out) {		// nothing to sent
		SciaRegs.SCICTL2.bit.TXINTENA = 0;			// disable TX interrupt
		return;
	}
	SciaRegs.SCITXBUF.all = tx_buff[tx_out];
	ROLLOVER(tx_out, TX0_SIZE);

	SciaRegs.SCIFFTX.bit.TXFFINTCLR = 1;   // Clear SCI Interrupt flag
	PieCtrlRegs.PIEACK.bit.ACK9 = 1;       // Issue PIE ACK
}

int utx0_ready(void) {
	int i = tx_in;

	ROLLOVER(i, TX0_SIZE);
	return vu16(tx_out) ^ i;		// 0 = busy
}

void uputchar0(const char c) {
	int i = tx_in;

	ROLLOVER(i, TX0_SIZE);
	tx_buff[tx_in] = c;
	while (i == vu16(tx_out));;		// until at least one byte free
	// tx_out modified by interrupt !
	tx_in = i;
//	UTX0_IEN = 1;                         // enable TX interrupt
	SciaRegs.SCICTL2.bit.TXINTENA = 1;
}

void uputs0(const char *s) {
	while (*s)
		uputchar0(*s++);
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

char txBuff[16];    // Send data for SCI-A
char rxBuff[16];    // Received data for SCI-A

static rbd_t _rbdRx;
static rbd_t _rbdTx;
static char _rbmemRx[8];
static char _rbmemTx[8];

/* ~~~~~~~~~~~~~~~~~~~~~  Private function prototypes  ~~~~~~~~~~~~~~~~~~~~~ */
void scia_fifo_config(const uint32_t baudrate, short rxIntEn, short txIntEn);

/* ~~~~~~~~~~~~~~~~~~~  Private function implementations  ~~~~~~~~~~~~~~~~~~ */

interrupt void sciaTxFifoIsr(void) {
	volatile uint16_t err = 0;

	for (short i = 0; i < SCI_HW_TX_FIFO_DEPTH; i++) {
		err = ring_buffer_get(_rbdTx, (uint16_t *) &SciaRegs.SCITXBUF.all);
		while (!SciaRegs.SCICTL2.bit.TXRDY)
			;;
	}

	SciaRegs.SCIFFTX.bit.TXFFINTCLR = 1;   // Clear SCI Interrupt flag
	PieCtrlRegs.PIEACK.bit.ACK9 = 1;       // Issue PIE ACK
}

interrupt void sciaRxFifoIsr(void) {
	volatile uint16_t err = 0;

	for (short i = 0; i < SCI_HW_RX_FIFO_DEPTH; i++)
		err = ring_buffer_put(_rbdRx, (uint16_t *) &SciaRegs.SCIRXBUF.all);

	SciaRegs.SCIFFRX.bit.RXFFOVRCLR = 1;   // Clear Overflow flag
	SciaRegs.SCIFFRX.bit.RXFFINTCLR = 1;   // Clear Interrupt flag

	PieCtrlRegs.PIEACK.bit.ACK9 = 1;       // Issue PIE ack

//	char fromRingBuff[10];
//	int rxCnt = ring_buffer_get(_rbdRx, &fromRingBuff);
//	MD_SCI_puts(&fromRingBuff[0]);
}

void MD_SCI_puts(const char * str) {
	ring_buffer_put(_rbdTx, (char *) str);
	SciaRegs.SCIFFTX.bit.TXFFINTCLR = 1;   // Clear SCI Interrupt flag
	SciaRegs.SCICTL2.bit.TXINTENA = 1;
}

void MD_SCI_init(const uint32_t baudrate) {
//	GPIO_SetupPinMux(42, GPIO_MUX_CPU1, 1);
//	GPIO_SetupPinOptions(42, GPIO_INPUT, GPIO_PUSHPULL);
//	GPIO_SetupPinMux(43, GPIO_MUX_CPU1, 1);
//	GPIO_SetupPinOptions(43, GPIO_OUTPUT, GPIO_ASYNC);

	GpioCtrlRegs.GPBMUX1.bit.GPIO42 = 3;
	GpioCtrlRegs.GPBMUX1.bit.GPIO43 = 3;
	GpioCtrlRegs.GPBGMUX1.bit.GPIO42 = 3;
	GpioCtrlRegs.GPBGMUX1.bit.GPIO43 = 3;
	// Interrupts that are used in this example are re-mapped to
	// ISR functions found within this file.
	EALLOW;
	// This is needed to write to EALLOW protected registers
	PieVectTable.SCIA_RX_INT = &sciaRxFifoIsr;
	PieVectTable.SCIA_TX_INT = &sciaTxFifoIsr;
	EDIS;
	// This is needed to disable write to EALLOW protected registers

	scia_fifo_config(baudrate, 0, 0);

	/* Initialize the ring buffer */
	rb_attr_t attrRx =
		        { sizeof(_rbmemRx[0]), sizeof(_rbmemRx) / sizeof(_rbmemRx[0]),
		                _rbmemRx };

	rb_attr_t attrTx =
		        { sizeof(_rbmemTx[0]), sizeof(_rbmemTx) / sizeof(_rbmemTx[0]),
		                _rbmemTx };

	ring_buffer_init(&_rbdRx, &attrRx);
	ring_buffer_init(&_rbdTx, &attrTx);
}

void scia_fifo_config(const uint32_t baudrate, short rxIntEn, short txIntEn) {
	SciaRegs.SCICCR.all = 0x0007;      // 1 stop bit,  No loopback
	// No parity,8 char bits,
	// async mode, idle-line protocol
	SciaRegs.SCICTL1.all = 0x0003;     // enable TX, RX, internal SCICLK,
	// Disable RX ERR, SLEEP, TXWAKE
	SciaRegs.SCICTL2.bit.TXINTENA = (int) txIntEn;
	SciaRegs.SCICTL2.bit.RXBKINTENA = (int) rxIntEn;
	SciaRegs.SCIHBAUD.all = 0x0000;
	SciaRegs.SCILBAUD.all = LSPCLK_DEFAULT / (baudrate * 8) - 1;

	SciaRegs.SCICCR.bit.LOOPBKENA = 0;  // Enable loop back
	SciaRegs.SCIFFTX.all = 0xC022;
	SciaRegs.SCIFFRX.all = 0x0022;
	SciaRegs.SCIFFCT.all = 0x00;

	SciaRegs.SCICTL1.all = 0x0023;     // Relinquish SCI from Reset
	SciaRegs.SCIFFTX.bit.TXFIFORESET = 1;
	SciaRegs.SCIFFRX.bit.RXFIFORESET = 1;
}

