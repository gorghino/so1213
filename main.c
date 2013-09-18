/*  Copyright (C) 2013 Aguiari Davide, Giacò Jacopo, Trotta Marco
 *  Authors: Aguiari Davide davide.aguiari@studio.unibo.it
 *        	 Giacò Jacopo jacopo.giaco@studio.unibo.it
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

#include "handler.h"
#include "pcb.e"
#include "asl.e"
#include "utils.h"
#include "libumps.h"
#include "const13.h"
#include "uMPStypes.h"
#include "print.h"
#include "const13_customized.h"
#include "main.h"

#define NUM_DEVICES 8

pcb_t *ready_queue[MAX_CPUS];
pcb_t *current_process[MAX_CPUS];
// Conta quanti processi nella coda ready della CPU
int process_count[MAX_CPUS];
int softBlock_count;
int stateCPU[MAX_CPUS]; 	


int main(){
	int i = 0;

	((state_t *)INT_NEWAREA)->pc_epc = ((state_t *)INT_NEWAREA)->reg_t9 = (memaddr)interruptHandler;
	((state_t *)INT_NEWAREA)->reg_sp = RAMTOP;
	((state_t *)INT_NEWAREA)->status &= ~(STATUS_IEc|STATUS_KUc|STATUS_VMc);
	((state_t *)INT_NEWAREA)->status |= STATUS_TE;

	((state_t *)TLB_NEWAREA)->pc_epc = ((state_t *)TLB_NEWAREA)->reg_t9 = (memaddr)tlbHandler;
	((state_t *)TLB_NEWAREA)->reg_sp = RAMTOP;
	((state_t *)TLB_NEWAREA)->status &= ~(STATUS_IEc|STATUS_KUc|STATUS_VMc);
	((state_t *)INT_NEWAREA)->status |= STATUS_TE;

	((state_t *)PGMTRAP_NEWAREA)->pc_epc = ((state_t *)PGMTRAP_NEWAREA)->reg_t9 = (memaddr)trapHandler;
	((state_t *)PGMTRAP_NEWAREA)->reg_sp = RAMTOP;
	((state_t *)PGMTRAP_NEWAREA)->status &= ~(STATUS_IEc|STATUS_KUc|STATUS_VMc);
	((state_t *)INT_NEWAREA)->status |= STATUS_TE;

	((state_t *)SYSBK_NEWAREA)->pc_epc = ((state_t *)SYSBK_NEWAREA)->reg_t9 = (memaddr)syscallHandler;
	((state_t *)SYSBK_NEWAREA)->reg_sp = RAMTOP;
	((state_t *)SYSBK_NEWAREA)->status &= ~(STATUS_IEc|STATUS_KUc|STATUS_VMc);
	((state_t *)INT_NEWAREA)->status |= STATUS_TE;


    //addokbuf("Popolo le aree\n");
    /*Populate the four New Areas in the ROM Reserved Frame. (See Section
		3.2.2-pops.) For each New processor state*/ 
    for (i=0; i<MAX_CPUS;i++){

	    /*Set the PC to the address of your nucleus function that is to handle
			exceptions of that type.*/
	    new_old_areas[i][1].pc_epc = new_old_areas[i][1].reg_t9 = (memaddr)interruptHandler;
	    new_old_areas[i][1].status &= ~(STATUS_IEc|STATUS_KUc|STATUS_VMc);
		new_old_areas[i][1].status |= STATUS_TE;

	    new_old_areas[i][3].pc_epc = new_old_areas[i][3].reg_t9 = (memaddr)tlbHandler;
	    new_old_areas[i][3].status &= ~(STATUS_IEc|STATUS_KUc|STATUS_VMc);
		new_old_areas[i][3].status |= STATUS_TE;

	    new_old_areas[i][5].pc_epc = new_old_areas[i][5].reg_t9 = (memaddr)trapHandler;
	    new_old_areas[i][5].status &= ~(STATUS_IEc|STATUS_KUc|STATUS_VMc);
		new_old_areas[i][5].status |= STATUS_TE;

	    new_old_areas[i][7].pc_epc = new_old_areas[i][7].reg_t9 = (memaddr)syscallHandler;
	    new_old_areas[i][7].status &= ~(STATUS_IEc|STATUS_KUc|STATUS_VMc);
		new_old_areas[i][7].status |= STATUS_TE;

		if(i == 0){
			new_old_areas[i][1].reg_sp = RAMTOP;
			new_old_areas[i][3].reg_sp = RAMTOP;	
			new_old_areas[i][5].reg_sp = RAMTOP;	
			new_old_areas[i][7].reg_sp = RAMTOP;
		}
		else{
			new_old_areas[i][1].reg_sp = RAMTOP - (FRAME_SIZE * i);
			new_old_areas[i][3].reg_sp = RAMTOP - (FRAME_SIZE * i);	
			new_old_areas[i][5].reg_sp = RAMTOP - (FRAME_SIZE * i);
			new_old_areas[i][7].reg_sp = RAMTOP - (FRAME_SIZE * i);
		}
    }


    /*Initialize the Level 2 (phase 1 - see Chapter 2) data structures:*/
    //addokbuf("Inizializzo liste fase1\n");
    initPcbs();
    initASL();


    /*Initialize all nucleus maintained variables: Process Count, Soft-block Count,
		Ready Queue, and Current Process.*/
	//addokbuf("Inizializzo strutture dati\n");

	for (i=0; i<MAX_CPUS;i++){
    	ready_queue[i] = NULL; /*Puntatore alla testa della ready Queue*/
    	process_count[i] = 0;  
    		
    }
    pseudo_clock = 0; 

    /*Initialize all nucleus maintained semaphores. 
    	In addition to the above nucleus variables, there is one semaphore variable for each external (sub)device
		in μMPS2, plus a semaphore to represent a pseudo-clock timer. Since terminal devices are 
		actually two independent sub-devices (see Section 5.7pops), the nucleus maintains 
		two semaphores for each terminal device. All of these semaphores need to be initialized to zero.*/

	//addokbuf("Inizializzo semafori\n");

	int j = 0;
	for(j=0;j<NUM_DEVICES;j++){
		sem_disk[j] = 0;
		sem_tape[j] = 0;
		sem_ethernet[j] = 0;
		sem_printer[j] = 0;
		sem_terminal_read[j] = 0;
		sem_terminal_write[j] = 0;
	}

	//semArray[0] = semPV
	//semArray[1] = semClock
	//semArray[2] = semScheduler
	for(j=0;j<3;j++)
		semArray[j] = 1;


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


	/*Call the scheduler*/
	init();
	return -1;

}