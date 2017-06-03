/**
 * @file        main.h
 * @project		_md_driverlib_TEMPLATE
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
#ifndef ADCEPWMSOC_DCLCLAPID_H_
#define ADCEPWMSOC_DCLCLAPID_H_

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
#include "F28x_Project.h"
#include "F2837xD_Cla_defines.h"		//!< Control law accelerator
#include "md_types.h"

void MD_ADC_init(const uint32_t ADCx_BASE, ADC_Resolution resolution,
                 ADC_ClkPrescale clk_div);

void MD_EPWM_init(const uint32_t EPWMx_BASE, float samplePeriode,
                  EPWM_ADCStartOfConversionType SOC_AB,
                  EPWM_CounterCompareModule CounterCompareModule, uint16_t preScaleCount);

void MD_DAC_init(const uint32_t DACx_BASE, const uint16_t initialOut);

void MD_ADC_SOC_config(const uint32_t ADCx_BASE, ADC_Channel CH_ADCINx,
                       ADC_IntNumber INT_NUMBERx, ADC_SOCNumber SOCx,
                       uint32_t sampleWindow_ns);

ADC_Resolution ADC_getResolution(const uint32_t ADCx_BASE);
ADC_SignalMode ADC_getSignalMode(const uint32_t ADCx_BASE);

int cmdIfaceExec(const char *cmd);
void cmdIfaceHandler(void);


__interrupt void Cla1Task1();
__interrupt void Cla1Task2();
__interrupt void Cla1Task3();
__interrupt void Cla1Task4();
__interrupt void Cla1Task5();
__interrupt void Cla1Task6();
__interrupt void Cla1Task7();
__interrupt void Cla1Task8();

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~  Public typedefs  ~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~  Public variable definitions  ~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~  Public function prototypes  ~~~~~~~~~~~~~~~~~~~~~ */
//__interrupt void IRQ_ADC_A1(void);

//void MD_EPWM_init(const uint32_t EPWMx_BASE, const uint32_t samplePeriod_us);
//void MD_ADC_init(const uint32_t ADCx_BASE, ADC_Channel CH_ADCINx, ADC_IntNumber INTx,
//                 ADC_SOCNumber SOCx, uint32_t sampleWindow_ns);
//void MD_ADCA_init_mm(ADC_Channel CH_ADCINx, uint32_t sampleWindow_ns);
//
//void MD_DAC_init(const uint32_t DACx_BASE, const uint16_t initialOut);

#ifdef __cplusplus
}
#endif

#endif
