/**
 * @file        md_cla.h
 * @project		06_adcEpwmSoc_dclClaPid
 * 
 * @date        3 Jun 2017
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
#ifndef MD_CLA_H_
#define MD_CLA_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  Includes  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "F2837xD_Cla_defines.h"		//!< Control law accelerator
#include "cla_shared.h"

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~  Configuration  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~  Public typedefs  ~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~~~  Public macro definitions  ~~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~  Public variable definitions  ~~~~~~~~~~~~~~~~~~~~~ */
__interrupt void CLA1_PID1_calc();
__interrupt void Cla1Task2();
__interrupt void Cla1Task3();
__interrupt void Cla1Task4();
__interrupt void Cla1Task5();
__interrupt void Cla1Task6();
__interrupt void Cla1Task7();
__interrupt void Cla1Task8();

/* ~~~~~~~~~~~~~~~~~~~~~~  Public function prototypes  ~~~~~~~~~~~~~~~~~~~~~ */
void CLA_configClaMemory(void);
void CLA_initCpu1Cla1(void);

__interrupt void cla1Isr1();
__interrupt void cla1Isr2();
__interrupt void cla1Isr3();
__interrupt void cla1Isr4();
__interrupt void cla1Isr5();
__interrupt void cla1Isr6();
__interrupt void cla1Isr7();
__interrupt void cla1Isr8();

#ifdef __cplusplus
}
#endif

#endif
