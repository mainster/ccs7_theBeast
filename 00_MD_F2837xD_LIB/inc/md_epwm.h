/**
 * @file        md_epwm.h
 * @project		00_MD_F2837xD_LIB
 * 
 * @date        May 29, 2017
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
#ifndef MD_EPWM_H_
#define MD_EPWM_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  Includes  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "driverlib.h"
#include "device.h"
/* ------------------------------------------------------------------------- */
#include "F28x_Project.h"
#include "F2837xD_GlobalPrototypes.h"

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~  Configuration  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~  Public typedefs  ~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
typedef enum {
	START = 0x01,
	STOP
} State_t;

/* ~~~~~~~~~~~~~~~~~~~~~~~  Public macro definitions  ~~~~~~~~~~~~~~~~~~~~~~ */
#define _TBPRD(tSample_us)	((uint32_t)(tSample_us * (DEVICE_LSPCLK_FREQ * 1e-6)/2))
#define _CMPA(tHigh_us)		((uint32_t)(tHigh_us * (DEVICE_LSPCLK_FREQ * 1e-6)))

#define ACT_IGNORE			0x00
#define ACT_FORCE_LOW		0x01
#define ACT_FORCE_HIGH		0x02
#define ACT_FORCE_TOGGLE	0x03

/* ~~~~~~~~~~~~~~~~~~~~~  Public variable definitions  ~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~~  Public function prototypes  ~~~~~~~~~~~~~~~~~~~~~ */
//__interrupt void IRQ_EPWM_1(void);

void MD_EPWM_init(const uint32_t EPWMx_BASE, const uint32_t samplePeriod_us);
void MD_EPWM1_init_mm(const uint32_t tSample_us);

void MD_EPWM1_acquisition(State_t state);


#ifdef __cplusplus
}
#endif

#endif
