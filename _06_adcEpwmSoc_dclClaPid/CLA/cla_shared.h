/**
 * @file        cla_shared.h
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
#ifndef CLA_CLA_SHARED_H_
#define CLA_CLA_SHARED_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  Includes  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
//#include "F2837xD_device.h"
#include "F2837xD_Cla_typedefs.h"
#include "F2837xD_Cla_defines.h"
#include <stdint.h>
#include "md_types.h"
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~  Public typedefs  ~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~~~  Public macro definitions  ~~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~  Public variable definitions  ~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~~  Public function prototypes  ~~~~~~~~~~~~~~~~~~~~~ */

//!< Task 1 (C) Variables
#define   OUTPUT_LEN    INPUT_LEN+FILTER_LEN
#define   NUM_SAMPLES   128
#define   INPUT_LEN     128
#define   FILTER_LEN    5

#define   ADC_BUF_LEN	256


extern PID_t	pid1;
extern uint16_t adcaRes[];
extern uint16_t SampleCount;
extern uint16_t resultsIndex;

//!< Task 2 (C) Variables
//!< Task 3 (C) Variables
//!< Task 4 (C) Variables
//!< Task 5 (C) Variables
//!< Task 6 (C) Variables
//!< Task 7 (C) Variables
//!< Task 8 (C) Variables
//!< Common (C) Variables

//!< Function Prototypes
//!
//! The following are symbols defined in the CLA (assembly)
//! code Including them in the shared header file makes them
//! .global and the main CPU can make use of them.
__interrupt void CLA1_PID1_calc();
__interrupt void Cla1Task2();
__interrupt void Cla1Task3();
__interrupt void Cla1Task4();
__interrupt void Cla1Task5();
__interrupt void Cla1Task6();
__interrupt void Cla1Task7();
__interrupt void Cla1Task8();

#ifdef __cplusplus
}
#endif

#endif


