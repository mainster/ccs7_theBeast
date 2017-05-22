/**
 * @file        md_gpio.h
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
#ifndef MD_GPIO_H_
#define MD_GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  Includes  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "../md_theBeast/md_globals.h"
#include "driverlib.h"
#include "pin_map.h"
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~  Configuration  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#define LAUNCHPAD_LED_BU	31		// GPIO31
#define LAUNCHPAD_LED_RD	34		// GPIO34


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~  Public typedefs  ~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
typedef enum {
	GPIO_DRIVER_TYPE_PP = 0x00,
	GPIO_DRIVER_TYPE_PU,
	GPIO_DRIVER_TYPE_OD,
} GPIO_DriverType;

typedef enum {
	GPIO_LOGIC_INVERT = 0,
	GPIO_LOGIC_STD,
} GPIO_LogicType;

typedef enum {
	GPIO_OFF = 0,
	GPIO_ON,
} GPIO_PinState;

typedef uint32_t GPIO_AlternFunc;
typedef uint16_t GPIO_PinNumber;

typedef struct {
	GPIO_PinNumber		Gpiox;
	GPIO_CoreSelect 	MasterCore;
	GPIO_Direction 		Direction;
	GPIO_AlternFunc		AlternFunc;
	GPIO_DriverType		DriverType;
	GPIO_LogicType		LogicType;
	GPIO_PinState		PorState;
} MD_GPIO_t;

extern MD_GPIO_t GPIO_config[];

/* ~~~~~~~~~~~~~~~~~~~~~~~  Public macro definitions  ~~~~~~~~~~~~~~~~~~~~~~ */

#define MD_GPIO_INIT(x, c, d, a, o, l, i) \
		GPIO_setMasterCore (x, c); \
		GPIO_setDirectionMode (x, d); \
		GPIO_setPinConfig (a); \
		GPIO_setPadConfig (x, o|l); \
		GPIO_writePin (x, i);


#define LED_BL_OFF	GPIO_setPortPins(GPIO_PORT_A, (1 << LAUNCHPAD_LED_BL);
#define LED_RD_OFF	GPIO_setPortPins(GPIO_PORT_B, (1 << LAUNCHPAD_LED_RD);
#define LED_BL_ON	GPIO_clearPortPins(GPIO_PORT_A, (1 << LAUNCHPAD_LED_BL);
#define LED_RD_ON	GPIO_clearPortPins(GPIO_PORT_B, (1 << LAUNCHPAD_LED_RD);


/* ~~~~~~~~~~~~~~~~~~~~~  Public variable definitions  ~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~~  Public function prototypes  ~~~~~~~~~~~~~~~~~~~~~ */
int MD_GPIO_Setup(MD_GPIO_t *conf, const uint16_t length);

#ifdef __cplusplus
}
#endif

#endif
