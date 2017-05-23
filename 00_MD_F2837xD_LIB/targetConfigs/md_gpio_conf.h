
/**
 * @file        md_gpio_conf.h
 * @project		_A_dlib_bitfield_dcl
 *
 * @date        21 May 2017
 * @author      Manuel Del Basso (mainster)
 * @email       manuel.delbasso@gmail.com
 *
 * @ide         Code Composer Studio Version: 7.1.0.00015
 * @license		GNU GPL v3
 *
 * @brief       DESCRIPTION
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
/* -------------------------------  Includes  ------------------------------ */
#include "md_gpio.h"

/* ---------------------------  Private typedefs  -------------------------- */
extern MD_GPIO_TypeDef_t MD_GPIO_TypeDef;

/* ----------------------------  Private macros  --------------------------- */
#define GPIO_COUNT_IN_PACKED_INIT_STRUCT	2

#define LED_RD 31
#define LED_BU 34

/* ----------------------------  Configuration  ---------------------------- */
MD_GPIO_t MD_GPIO_config[] = {
		{ LED_BU, GPIO_CORE_CPU1, GPIO_DIR_MODE_OUT, GPIO_34_GPIO34, GPIO_DRIVER_TYPE_PP, GPIO_LOGIC_INVERT, GPIO_OFF },
		{ LED_RD, GPIO_CORE_CPU1, GPIO_DIR_MODE_OUT, GPIO_31_GPIO31, GPIO_DRIVER_TYPE_PP, GPIO_LOGIC_STD, GPIO_OFF }
};

/* --------------------------  Private functions  -------------------------- */

/* --------------------------  Private variables  -------------------------- */

/* ----------------------------  Private defines  -------------------------- */

/* ---------------------  Private function prototypes  --------------------- */

#ifdef __cplusplus
}
#endif

#endif
