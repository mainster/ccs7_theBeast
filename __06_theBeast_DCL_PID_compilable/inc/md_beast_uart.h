/**
 * @file        md_beast_uart.h
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
#ifndef MD_BEAST_UART_H_
#define MD_BEAST_UART_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "md_globals.h"

#define RX0_SIZE    10      // usable: RX0_SIZE + 2 (4 .. 258)
#define TX0_SIZE    8       // usable: TX0_SIZE + 1 (3 .. 257)

void init_uart0(const uint32_t baudrate) ;
int ukbhit0(void) ;
int ugetchar0(void) ;
interrupt void USART0_RX_vect(void);
interrupt void USART0_UDRE_vect(void);
int utx0_ready(void);
void uputchar0(const char c);
void uputs0(const char *s);


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  Includes  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "md_circ_buffer.h"
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~  Configuration  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~  Public typedefs  ~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~~~  Public macro definitions  ~~~~~~~~~~~~~~~~~~~~~~ */
#define SCI_HW_TX_FIFO_DEPTH	16
#define SCI_HW_RX_FIFO_DEPTH	16
/* ~~~~~~~~~~~~~~~~~~~~~  Public variable definitions  ~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~~  Public function prototypes  ~~~~~~~~~~~~~~~~~~~~~ */

// (LSPCLK) derived from SYSCLK. LSPCLK uses a /4 divider by default,
// but the ratio can be changed via the LOSPCP register
#define LSPCLK_DEFAULT	(SYSCLKOUT / 4)
//#define SCI_FREQ        100E3
//#define SCI_PRD         (LSPCLK_FREQ/(SCI_FREQ*8))-1

extern char txBuff[];    // Send data for SCI-A
extern char rxBuff[];    // Received data for SCI-A

// Function Prototypes
interrupt void sciaTxFifoIsr(void);
interrupt void sciaRxFifoIsr(void);

//void scia_uart_init(const uint32_t baudrate);
void scia_fifo_config(const uint32_t baudrate, short rxIntEn, short txIntEn);
void MD_SCI_init(const uint32_t baudrate);
char MD_SCI_getc(void);
void MD_SCI_putc(const char c);
void MD_SCI_puts(const char * str);

#ifdef __cplusplus
}
#endif

#endif
