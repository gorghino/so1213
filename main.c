/*  Copyright (C) 2013 Aguiari Davide, Giacò Jacopo, Trotta Marco
 *  Authors: Aguiari Davide davide.aguiari@studio.unibo.it
 *    		 Giacò Jacopo jacopo.giaco@studio.unibo.it
 					Trotta Marco marco.trotta2@studio.unibo.it
 							
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "main.h"
#include "scheduler.h"
#include "handler.h"
#include "pcb.e"
#include "asl.e"
#include "utils.h"
#include "libumps.h"
#include "const13.h"
#include "uMPStypes.h"
#include "print.h"


#define	MAX_CPUS 2
#define NUM_DEVICES 8




void main(){
	int i = 0;

    state_t *new_old_areas[MAX_CPUS][8];	

    addokbuf("Popolo le aree\n");
    /*Populate the four New Areas in the ROM Reserved Frame. (See Section
		3.2.2-pops.) For each New processor state*/ 
    for (i=0; i<MAX_CPUS;i++){
	    new_old_areas[i][0] = (state_t *) INT_NEWAREA;
	    new_old_areas[i][1] = (state_t *) INT_OLDAREA;
	    new_old_areas[i][2] = (state_t *) TLB_NEWAREA;
	    new_old_areas[i][3] = (state_t *) TLB_OLDAREA;
	    new_old_areas[i][4] = (state_t *) PGMTRAP_NEWAREA;
	    new_old_areas[i][5] = (state_t *) PGMTRAP_OLDAREA;
	    new_old_areas[i][6] = (state_t *) SYSBK_NEWAREA;
	    new_old_areas[i][7] = (state_t *) SYSBK_OLDAREA;

	    /*Set the PC to the address of your nucleus function that is to handle
			exceptions of that type.*/
	    new_old_areas[i][0]->pc_epc = new_old_areas[i][0]->reg_t9 = (memaddr)interruptHandler;
	    new_old_areas[i][2]->pc_epc = new_old_areas[i][2]->reg_t9 = (memaddr)tlbHandler;
	    new_old_areas[i][4]->pc_epc = new_old_areas[i][4]->reg_t9 = (memaddr)trapHandler;
	    new_old_areas[i][6]->pc_epc = new_old_areas[i][6]->reg_t9 = (memaddr)syscallHandler;

	    /*Set the Status register to mask all interrupts, turn virtual memory off,
			enable the processor Local Timer, and be in kernel-mode.*/

	    /*IEc: bit 0 - The “current” global interrupt enable bit. When 0, regardless
			of the settings in Status.IM all external interrupts are disabled. When 1,
			external interrupt acceptance is controlled by Status.IM.
		KUc: bit 1 - The “current” kernel-mode user-mode control bit. When Sta-
			tus.KUc=0 the processor is in kernel-mode.
		VMc: Bit 24 - The “current” VM on/off flag bit. Status.VMc=0 indicates
			that virtual memory translation is currently off
		TE: Bit 27 - the processor Local Timer enable bit. A 1-bit mask that en-
			ables/disables the processor’s Local Timer. See Section 5.2.2 for more in-
			formation about this timer.*/

		new_old_areas[i][0]->status &= ~(STATUS_IEc|STATUS_KUc|STATUS_VMc);
		new_old_areas[i][0]->status |= STATUS_TE;
		new_old_areas[i][2]->status &= ~(STATUS_IEc|STATUS_KUc|STATUS_VMc);
		new_old_areas[i][2]->status |= STATUS_TE;
		new_old_areas[i][4]->status &= ~(STATUS_IEc|STATUS_KUc|STATUS_VMc);
		new_old_areas[i][4]->status |= STATUS_TE;
		new_old_areas[i][6]->status &= ~(STATUS_IEc|STATUS_KUc|STATUS_VMc);
		new_old_areas[i][6]->status |= STATUS_TE;

		/*Set the $SP to RAMTOP. Each exception handler will use the last
			frame of RAM for its stack.*/
		new_old_areas[i][0]->reg_sp = RAMTOP;
		new_old_areas[i][2]->reg_sp = RAMTOP;	
		new_old_areas[i][4]->reg_sp = RAMTOP;	
		new_old_areas[i][6]->reg_sp = RAMTOP;	
    }


    /*Initialize the Level 2 (phase 1 - see Chapter 2) data structures:*/
    addokbuf("Inizializzo liste fase1\n");
    initPcbs();
    initASL();


    /*Initialize all nucleus maintained variables: Process Count, Soft-block Count,
		Ready Queue, and Current Process.*/

	addokbuf("Inizializzo strutture dati\n");
	int process_count = 1;
    int softBlock_count;
    pcb_t *ready_queue = NULL; /*Puntatore alla testa della ready Queue*/
    pcb_t *current_process;

    /*Initialize all nucleus maintained semaphores. 
    	In addition to the above nucleus variables, there is one semaphore variable for each external (sub)device
		in μMPS2, plus a semaphore to represent a pseudo-clock timer. Since terminal devices are 
		actually two independent sub-devices (see Section 5.7pops), the nucleus maintains 
		two semaphores for each terminal device. All of these semaphores need to be initialized to zero.*/

	addokbuf("Inizializzo semafori\n");
	semd_t *semd_disk[8];
	semd_t *semd_tape[8];
	semd_t *semd_ethernet[8];
	semd_t *semd_printer[8];
	semd_t *semd_terminal_read[8];
	semd_t *semd_terminal_write[8];

	int j = 0;
	for(j=0;j<NUM_DEVICES;j++){
		semd_disk[j] = 0;
		semd_tape[j] = 0;
		semd_ethernet[j] = 0;
		semd_printer[j] = 0;
		semd_terminal_read[j] = 0;
		semd_terminal_write[j] = 0;
	}

	semd_t *pseudo_clock = 0;

	/*Instantiate a single process and place its ProcBlk in the Ready Queue. A
		process is instantiated by allocating a ProcBlk (i.e. allocPcb()), and
		initializing the processor state that is part of the ProcBlk. In particular this
		process needs to have interrupts enabled, virtual memory off, the processor
		Local Timer enabled, kernel-mode on, $SP set to RAMTOP-FRAMESIZE
		(i.e. use the penultimate RAM frame for its stack), and its PC set to the
		address of test. Test is a supplied function/process that will help you
		debug your nucleus. One can assign a variable (i.e. the PC) the address of
		a function by using
		. . . = (memaddr)test
		where memaddr, in TYPES. H has been aliased to unsigned int.
		Remember to declare the test function as “external” in your program by
		including the line:
		extern void test();
		For rather technical reasons that are somewhat explained in Section 8.2-
		pops, whenever one assigns a value to the PC one must also assign the
		same value to the general purpose register t9. (a.k.a. s t9 as defined in
		TYPES. H .) Hence this will be done when initializing the four New Areas as
		well as the processor state that defines this single process.*/

	addokbuf("Alloco pcb\n");
	pcb_t *new_process = allocPcb();

	new_process->p_s.status |= STATUS_IEc|STATUS_TE|STATUS_KUc;
	new_process->p_s.status &= ~STATUS_VMc;
	new_process->p_s.reg_sp = RAMTOP-FRAME_SIZE;
	new_process->p_s.pc_epc = new_process->p_s.reg_t9 = (memaddr)test; /*p2test*/

	insertProcQ(&ready_queue, new_process);
	process_count++;

	/*Call the scheduler*/
	scheduler(process_count, softBlock_count, &ready_queue, new_process);
}

