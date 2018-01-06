/**
 * @file        md_cla.c
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
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  Includes  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include <md_cla.h>
#include "F28x_Project.h"
#include "F2837xD_GlobalPrototypes.h"	//!< DSP bit field
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~  Private typedefs  ~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~  Private macro definitions  ~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~  Private variable declaration  ~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~  Private function prototypes  ~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~  Private function implementations  ~~~~~~~~~~~~~~~~~~ */
 
/**
 * CLA_initCpu1Cla1 - Initialize CLA1 task vectors and end of task interrupts
 */
void CLA_initCpu1Cla1(void) {
	// Compute all CLA task vectors
	// On Type-1 CLAs the MVECT registers accept full 16-bit task addresses as
	// opposed to offsets used on older Type-0 CLAs
	EALLOW;
	Cla1Regs.MVECT1 = (uint16_t) (&CLA1_PID1_calc);
	Cla1Regs.MVECT2 = (uint16_t) (&Cla1Task2);
	Cla1Regs.MVECT3 = (uint16_t) (&Cla1Task3);
	Cla1Regs.MVECT4 = (uint16_t) (&Cla1Task4);
	Cla1Regs.MVECT5 = (uint16_t) (&Cla1Task5);
	Cla1Regs.MVECT6 = (uint16_t) (&Cla1Task6);
	Cla1Regs.MVECT7 = (uint16_t) (&Cla1Task7);
	Cla1Regs.MVECT8 = (uint16_t) (&Cla1Task8);

	// Enable the IACK instruction to start a task on CLA in software
	// for all  8 CLA tasks. Also, globally enable all 8 tasks (or a
	// subset of tasks) by writing to their respective bits in the
	// MIER register
	Cla1Regs.MCTL.bit.IACKE = 1;
	Cla1Regs.MIER.all = (M_INT8 | M_INT7);

	// Configure the vectors for the end-of-task interrupt for all
	// 8 tasks
	PieVectTable.CLA1_1_INT = &cla1Isr1;
	PieVectTable.CLA1_2_INT = &cla1Isr2;
	PieVectTable.CLA1_3_INT = &cla1Isr3;
	PieVectTable.CLA1_4_INT = &cla1Isr4;
	PieVectTable.CLA1_5_INT = &cla1Isr5;
	PieVectTable.CLA1_6_INT = &cla1Isr6;
	PieVectTable.CLA1_7_INT = &cla1Isr7;
	PieVectTable.CLA1_8_INT = &cla1Isr8;

	// Set the adca.1 as the trigger for task 7
	DmaClaSrcSelRegs.CLA1TASKSRCSEL2.bit.TASK7 = 1;

	// Enable CLA interrupts at the group and subgroup levels
	PieCtrlRegs.PIEIER11.all = 0xFFFF;
	IER |= (M_INT11);
	EDIS;
}

/**
 * CLA_configClaMemory - Configure CLA memory sections
 */
void CLA_configClaMemory(void) {
	extern uint32_t Cla1funcsRunStart, Cla1funcsLoadStart, Cla1funcsLoadSize;

	EALLOW;
#ifdef _FLASH
	// Copy over code from FLASH to RAM
	memcpy((uint32_t *) &Cla1funcsRunStart, (uint32_t *) &Cla1funcsLoadStart,
	        (uint32_t) &Cla1funcsLoadSize);
#endif //_FLASH

	// Initialize and wait for CLA1ToCPUMsgRAM
	MemCfgRegs.MSGxINIT.bit.INIT_CLA1TOCPU = 1;
	while (MemCfgRegs.MSGxINITDONE.bit.INITDONE_CLA1TOCPU != 1) {
	};

	// Initialize and wait for CPUToCLA1MsgRAM
	MemCfgRegs.MSGxINIT.bit.INIT_CPUTOCLA1 = 1;
	while (MemCfgRegs.MSGxINITDONE.bit.INITDONE_CPUTOCLA1 != 1) {
	};

	// Select LS4RAM and LS5RAM to be the programming space for the CLA
	// First configure the CLA to be the master for LS4 and LS5 and then
	// set the space to be a program block
	MemCfgRegs.LSxMSEL.bit.MSEL_LS4 = 1;
	MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS4 = 1;
	MemCfgRegs.LSxMSEL.bit.MSEL_LS5 = 1;
	MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS5 = 1;

	EDIS;
}

__interrupt void cla1Isr1(void) {
	asm(" ESTOP0");
}		//!< cla1Isr1 - CLA1 ISR 1
__interrupt void cla1Isr2(void) {
	asm(" ESTOP0");
}		//!< cla1Isr2 - CLA1 ISR 2
__interrupt void cla1Isr3(void) {
	asm(" ESTOP0");
}		//!< cla1Isr3 - CLA1 ISR 3
__interrupt void cla1Isr4(void) {
	asm(" ESTOP0");
}		//!< cla1Isr4 - CLA1 ISR 4
__interrupt void cla1Isr5(void) {
	asm(" ESTOP0");
}		//!< cla1Isr5 - CLA1 ISR 5
__interrupt void cla1Isr6(void) {
	asm(" ESTOP0");
}		//!< cla1Isr6 - CLA1 ISR 6

/**
 * Post process after PID calculation.
 */
__interrupt void cla1Isr7(void) {
	//!< Clear the ADC interrupt flag so the next SOC can occur
	//!< Clear the PIEACK bits so another interrupt can be taken
	AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;  //make sure INT1 flag is cleared
	PieCtrlRegs.PIEACK.all = (PIEACK_GROUP1 | PIEACK_GROUP11);

	//!< Read the raw ADC RESULT1 register value and put it into the AdcBuf buffer
	// This can be compared to the CLA filtered value
//	adcaRes[SampleCount] = AdcaResultRegs.ADCRESULT0;

	// Read the CLA calculated value and put it in the
	// AdcFiltBuf buffer
//	adcFilRes[SampleCount++] = voltFilt;

	// Make sure that the buffer does not overflow
	// the buffer size.  If it is larger than ADC_BUF_LEN
	// then rewind back to the sample 0
//	if (SampleCount == ADC_BUF_LEN) {
//		SampleCount = 0;
	//    asm(" ESTOP0");
//	}

	// Uncomment to halt debugger here
}
__interrupt void cla1Isr8(void) {
	// Acknowledge the end-of-task interrupt for task 8
	PieCtrlRegs.PIEACK.all = M_INT11;

	// Uncomment to halt debugger here
	//    asm(" ESTOP0");
}

