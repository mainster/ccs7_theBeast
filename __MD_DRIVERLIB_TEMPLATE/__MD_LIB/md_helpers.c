/**
 * @file        md_helpers.c
 * @project		MD_F2837xD_LIB
 * 
 * @date        23 May 2017
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
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  Includes  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include <stdint.h>
#include <stdbool.h>
#include "md_helpers.h"

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~  Private typedefs  ~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~~  Private macro definitions  ~~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~  Private variable declaration  ~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~  Private function prototypes  ~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~  Private function implementations  ~~~~~~~~~~~~~~~~~~ */
/**
 * @brief      Simple digit to char converter.
 *
 * @param      number   The number
 * @param[in]  base     The base
 * @return     character representation of number
 */
char makedigit(uint32_t *number, uint32_t base) {
	static char map[] = "0123456789";
	uint16_t ix;

	for (ix = 0; *number >= base; ix++)
		*number -= base;

	return map[ix];
}

/**
 * @brief      Simple integral to string converter.
 *
 * @param[in]  number   The number
 * @return     Pointer to string representation of number.
 */
char *int2str(const uint32_t number, char *buff) {
	//	static char tmp[12] = { " " };
	//	char *p = &tmp[0];
	uint32_t _number = number;
	bool firstNonzero = false;

	if (!_number) {
		*buff++ = '0';
		*buff = '\0';
		return buff;
	}

	for (uint32_t ddec = 1e9; ddec >= 1; ddec /= 10) {
		*buff = makedigit(&_number, ddec);
		if ((*buff != '0') || firstNonzero) {
			firstNonzero = true;
			buff++;
		}
	}
	*buff = '\0';
	return buff;
}
 
