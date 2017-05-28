/**
 * @file        main.h
 * @project		_C_adc_ex2_soc_epwm
 * 
 * @date        23 May 2017
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
#ifndef MAIN_H_
#define MAIN_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  Includes  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~  Public macro definitions  ~~~~~~~~~~~~~~~~~~~~~~ */
//#define EX_ADC_RESOLUTION    ADC_RESOLUTION_12BIT   // Or ADC_RESOLUTION_16BIT
//#define EX_ADC_SIGNAL_MODE   ADC_MODE_SINGLE_ENDED  // Or ADC_MODE_DIFFERENTIAL
#include <stdint.h>
#include <stdbool.h>
#include "driverlib.h"
#include "device.h"
#include "DCL.h"

#define RESULTS_BUFFER_SIZE  256

#define REFERENCE_VDAC      0
#define REFERENCE_VREF      1
#define DACA         		1
#define DACB         		2
#define DACC         		3
#define REFERENCE           REFERENCE_VREF
#define DAC_NUM             DACA

#define BAUDRATE			230400U
#define	F_CPU				200000000U

// Globals
uint16_t adcAResults[RESULTS_BUFFER_SIZE];   // Buffer for results
volatile uint16_t bufferFull;                // Flag to indicate buffer is full
uint16_t index;                              // Index into result buffer


#define LED_RD(x)			GPIO_writePin(31, x)
#define LED_BL(x)			GPIO_writePin(34, x)
#define IO_SET_DBG(PIN, x)	GPIO_writePin(PIN, x)
#define IO_TGL_DBG(PIN)		GPIO_togglePin(PIN)

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~  Public typedefs  ~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~  Public variable definitions  ~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~  Public function prototypes  ~~~~~~~~~~~~~~~~~~~~~ */
__interrupt void IRQ_ADC_A1(void);
__interrupt void adcB2ISR(void);
__interrupt void IRQ_EPWM_1(void);

void MD_ADC_init(const uint32_t ADCx_BASE, ADC_Channel CH_ADCINx, ADC_IntNumber INTx,
                 ADC_SOCNumber SOCx, uint32_t sampleWindow_ns);

void MD_EPWM_init(const uint32_t EPWMx_BASE, const uint32_t samplePeriod_us);

void MD_DAC_config(uint16_t dac_num);
void MD_PID_instInit(PID *pidInst);



#ifdef __cplusplus
}
#endif

#endif
