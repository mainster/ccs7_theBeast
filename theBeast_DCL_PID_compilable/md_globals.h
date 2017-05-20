/**
 * @file        md_globals.h
 * @project		theBeast_DCL_PID_compilable
 * 
 * @date        May 20, 2017
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
#ifndef MD_GLOBALS_H_
#define MD_GLOBALS_H_

#ifdef __cplusplus
 extern "C" {
#endif

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  Includes  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include <DCL.h>
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~  Configuration  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~  Public typedefs  ~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~~~  Public macro definitions  ~~~~~~~~~~~~~~~~~~~~~~ */

 //			Optimization improvements

// always inline function x:

#define AIL(x)   static x __attribute__ ((always_inline)); static x


// never inline function x:

#define NIL(x)   x __attribute__ ((noinline)); x


// volatile access (reject unwanted removing access):

#define vu16(x) (*(volatile unsigned int*)&(x))
#define vs16(x) (*(volatile int*)&(x))
#define vu32(x) (*(volatile unsigned long*)&(x))
#define vs32(x) (*(volatile long*)&(x))

/* ~~~~~~~~~~~~~~~~~~~~~  Public variable definitions  ~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~~  Public function prototypes  ~~~~~~~~~~~~~~~~~~~~~ */
extern PID pid1;

#ifdef __cplusplus
 }
#endif

#endif
