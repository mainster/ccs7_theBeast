/**
 * @file        md_epwm.c
 * @project		00_MD_F2837xD_LIB
 * 
 * @date        May 29, 2017
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
/* ------------------------------------------------------------------------- */
#include "md_epwm.h"

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~  Private typedefs  ~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~~  Private macro definitions  ~~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~  Private variable declaration  ~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~  Private function prototypes  ~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~  Private function implementations  ~~~~~~~~~~~~~~~~~~ */
 
/**
 * Initialize EPWM module
 *
 * @param EPWMx_BASE
 * @param tSample_us
 */
void MD_EPWM_init(const uint32_t EPWMx_BASE, const uint32_t tSample_us) {
	//!< Freeze timer
	EPWM_setTimeBaseCounterMode(EPWMx_BASE, EPWM_COUNTER_MODE_STOP_FREEZE);
	SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

	//!< Disable SOCA trigger
	EPWM_disableADCTrigger(EPWMx_BASE, EPWM_SOC_A);

	//!< Configure the SOC to occur on the first up-count event
//	EPWM_setADCTriggerSource(EPWMx_BASE, EPWM_SOC_A, EPWM_SOC_TBCTR_U_CMPA);
//	EPWM_setADCTriggerEventPrescale(EPWMx_BASE, EPWM_SOC_A, 1);

	uint32_t
	regTBPRD = _TBPRD(tSample_us),
	regCMPA = _CMPA(tSample_us/10);

	EPWM_setTimeBasePeriod(EPWMx_BASE, regTBPRD);
	EPWM_setCounterCompareValue(EPWMx_BASE, EPWM_COUNTER_COMPARE_A, regCMPA);

	/**
	 * TBC run mode: UP_DOWN
	 * - Ignore event TBCTR == CMPA when in up counting direction
	 * - Do noting when TBCTR == TBPRD
	 * - Force EPWMxA pin high when TBCTR == CMPA in down counting direction
	 * - Force EPWMxA pin low when TBCTR == 0!
	 */
	EPwm1Regs.AQCTLA.bit.CAD = ACT_FORCE_HIGH;		//!< Event Compare A while down counting
	EPwm1Regs.AQCTLA.bit.CAU = ACT_IGNORE;			//!< Event Compare A while up counting
	EPwm1Regs.AQCTLA.bit.PRD = ACT_IGNORE;			//!< Event counter equals period
	EPwm1Regs.AQCTLA.bit.ZRO = ACT_FORCE_LOW;		//!< Event counter equals zero

	//!< Trigger ePWM interrupt when time-base counter equals zero
	EPWM_setInterruptSource(EPWMx_BASE, EPWM_INT_TBCTR_ZERO);

	//!< Enable ePWM interrupt.
	EPWM_enableInterrupt(EPWMx_BASE);

	SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);
}
