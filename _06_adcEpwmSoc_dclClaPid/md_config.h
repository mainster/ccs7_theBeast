/**
 * @file        md_config_TEMPLATE.h
 * @project		MD_F2833x_LIB
 * 
 * @date        26 Apr 2017
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
#ifndef MD_CONFIG_H_
#define MD_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------  Includes  ------------------------------ */
//#include "md"

/* ---------------------------  Private typedefs  -------------------------- */
/* ----------------------------  Private macros  --------------------------- */
#define GPIO_COUNT_IN_PACKED_INIT_STRUCT	4
#define GPIO_LED_RD 	31
#define GPIO_LED_BU 	34
#define GPIO_FS_OUT      0
#define GPIO_DBG16		16
#define ANA_A3		   111

/* ----------------------------  Configuration  ---------------------------- */
MD_GPIO_t MD_GPIO_config[GPIO_COUNT_IN_PACKED_INIT_STRUCT] =
	{
		{ GPIO_LED_BU, GPIO_CORE_CPU1, GPIO_DIR_MODE_OUT,
		GPIO_34_GPIO34, GPIO_DRIVER_TYPE_PP, GPIO_LOGIC_INVERT, GPIO_OFF },

		{ GPIO_LED_RD, GPIO_CORE_CPU1, GPIO_DIR_MODE_OUT,
		GPIO_31_GPIO31, GPIO_DRIVER_TYPE_PP, GPIO_LOGIC_STD, GPIO_OFF },

		{ GPIO_FS_OUT, GPIO_CORE_CPU1, GPIO_DIR_MODE_OUT,
		GPIO_0_EPWM1A, GPIO_DRIVER_TYPE_PU, GPIO_LOGIC_STD, GPIO_OFF },

		{ GPIO_DBG16, GPIO_CORE_CPU1, GPIO_DIR_MODE_OUT,
		GPIO_16_GPIO16, GPIO_DRIVER_TYPE_PU, GPIO_LOGIC_STD, GPIO_ON }
	};

/* --------------------------  Private functions  -------------------------- */

/* --------------------------  Private variables  -------------------------- */

/* ----------------------------  Private defines  -------------------------- */

/* ---------------------  Private function prototypes  --------------------- */

#ifdef __cplusplus
}
#endif

#endif
