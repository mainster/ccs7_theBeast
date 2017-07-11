/**
 * @file        md_iface.h
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
#ifndef MD_IFACE_H_
#define MD_IFACE_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  Includes  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~  Configuration  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#define   SCI_RX_BUFFER_SIZE 	16
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~  Public typedefs  ~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
static struct iface {
	char
	sciRxBuff[SCI_RX_BUFFER_SIZE],
	*pRxBuff,
	*rxCmdQue[10];
	short rxNewCmdsCtr;
	bool cmdRxBuffFull, cmdQueFull;
} i = {
		.pRxBuff = &i.sciRxBuff[0],
		.cmdRxBuffFull = false,
		.cmdQueFull = false,
		.rxNewCmdsCtr = 0
};
/* ~~~~~~~~~~~~~~~~~~~~~~~  Public macro definitions  ~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~  Public variable definitions  ~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~  Public function prototypes  ~~~~~~~~~~~~~~~~~~~~~ */

#ifdef __cplusplus
}
#endif

#endif
