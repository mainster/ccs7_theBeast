/**
 * @file        md_gpio.c
 * @project		_A_dlib_bitfield_dcl
 *
 * @date        21 May 2017
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
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  Includes  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include <md_gpio.h>
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~  Private typedefs  ~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~  Private macro definitions  ~~~~~~~~~~~~~~~~~~~~~~ */



//{
//		.Gpiox 		= LAUNCHPAD_LED_BL,
//		.MasterCore = GPIO_CORE_CPU1,
//		.Direction 	= GPIO_DIR_MODE_OUT,
//		.AlternFunc = GPIO_34_GPIO34,
//		.DriverType = GPIO_DRIVER_TYPE_PP,
//		.LogicType 	= GPIO_LOGIC_INVERT,
//		.PorState 	= GPIO_OFF
//},

/* ~~~~~~~~~~~~~~~~~~~~~  Private variable declaration  ~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~  Private function prototypes  ~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~  Private function implementations  ~~~~~~~~~~~~~~~~~~ */

int MD_GPIO_Setup(MD_GPIO_t *conf, uint16_t length) {
	for ( ; length > 0; length--) {
		MD_GPIO_INIT(
				conf->Gpiox, conf->MasterCore, conf->Direction, conf->AlternFunc,
				conf->DriverType, conf->LogicType, conf->PorState
				);
		conf++;
	}
	return 0;
}
