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
#include "md_globals.h"
#include "driverlib.h"
//#include "pin_map.h"
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~  Configuration  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#define LAUNCHPAD_LED_BU	31		// GPIO31
#define LAUNCHPAD_LED_RD	34		// GPIO34


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~  Public typedefs  ~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
enum GPIO_DriverType {
	GPIO_DRV_PP = 0,
	GPIO_DRV_PU,
	GPIO_DRV_OD,
};

enum GPIO_LogicType {
	GPIO_LOGIC_INVERT = 0,
	GPIO_LOGIC_NO_INV,
};

enum GPIO_PinState {
	GPIO_PIN_OFF = 0,
	GPIO_PIN_ON,
};

/* ------------------------------------------------------------------------- */
typedef GPIO_DriverType	 GPIO_DriverType_t;
typedef GPIO_LogicType   GPIO_LogicType_t;
typedef uint32_t  		 GPIO_AlternFunc_t;
typedef uint16_t  		 GPIO_PinNumber_t;

struct MD_GPIO {
	GPIO_PinNumber  Gpiox;
	GPIO_Direction  Direction;
	GPIO_CoreSelect  MasterCore;
	GPIO_AlternFunc  AlternFunc;
	GPIO_DriverType  DriverType;
	GPIO_LogicType  LogicType;
	GPIO_PinState  PorState;
};

extern MD_GPIO_t GPIO_config[];

/* ~~~~~~~~~~~~~~~~~~~~~~~  Public macro definitions  ~~~~~~~~~~~~~~~~~~~~~~ */
#define MD_GPIO_INIT(x, c, d, a, o, l, i) \
		GPIO_setMasterCore (x, c); \
		GPIO_setDirectionMode (x, d); \
		GPIO_setPinConfig (a); \
		GPIO_setPadConfig (x, o|l); \
		GPIO_writePin (x, i);

/* ~~~~~~~~~~~~~~~~~~~~~  Public variable definitions  ~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~  Public function prototypes  ~~~~~~~~~~~~~~~~~~~~~ */
int MD_GPIO_Setup(MD_GPIO_t *conf, const uint16_t length);


#ifdef __cplusplus
}
#endif

#endif
