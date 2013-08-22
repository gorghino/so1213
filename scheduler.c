/*  Copyright (C) 2013 Aguiari Davide, Giacò Jacopo, Trotta Marco
 *  Authors: Aguiari Davide davide.aguiari@studio.unibo.it
 *			 Giacò Jacopo jacopo.giaco@studio.unibo.it
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

#include "print.h"
#include "libumps.h"
#include "scheduler.h"
#include "pcb.e"
#include "main.h"


 /*Your nucleus should guarantee finite progress; consequently, every ready process
	will have an opportunity to execute. For simplicity’s sake this chapter describes
	the implementation of a simple round-robin scheduler with a time slice value of 5
	milliseconds.
	The scheduler also needs to perform some simple deadlock detection and if
	deadlock is detected perform some appropriate action; e.g. invoke the PANIC
	ROM service/instruction.
	We define the following:
	• Ready Queue: A (tail) pointer to a queue of ProcBlk’s representing pro-
	cesses that are ready and waiting for a turn at execution.
	• Current Process: A pointer to a ProcBlk that represents the current execut-
	ing process.
	• Process Count: The count of the number of processes in the system.
	• Soft-block Count: The number of processes in the system currently blocked
	and waiting for an interrupt; either an I/O to complete, or a timer to “expire.”
	The scheduler should behave in the following manner if the Ready Queue is
	empty:
	1. If the Process Count is zero invoke the HALT ROM service/instruction.
	2. Deadlock for Kaya is defined as when the Process Count > 0 and the Soft-
	block Count is zero. Take an appropriate deadlock detected action. (e.g.
	Invoke the PANIC ROM service/instruction.)
	3. If the Process Count > 0 and the Soft-block Count > 0 enter a Wait State.
	A Wait State is a state where the processor is “twiddling its thumbs,” or
	waiting until an interrupt to occur. μMPS2 supports a WAIT instruction ex-
	pressly for this purpose. See Section Section 6.2-pops for more information
	about the WAIT instruction.*/

#define	MAX_CPUS 1

extern void addokbuf(char *strp);
extern void itoa();
extern void strreverse();

extern pcb_t *current_process[MAX_CPUS];
extern pcb_t *ready_queue[MAX_CPUS];
extern int softBlock_count[MAX_CPUS];
extern state_t *new_old_areas[MAX_CPUS][8];	
// Conta quanti processi nella coda ready della CPU
extern int process_count[MAX_CPUS];

HIDDEN unsigned int pcb_Lock = 1;

state_t scheduler[MAX_CPUS];



void schedule(){
	int cpuID = getPRID();
	pcb_t *pRunning[MAX_CPUS];
	//addokbuf("SCHEDULER\n");

	if((pRunning[cpuID] = removeProcQ(&ready_queue[cpuID])) != NULL){
		forallProcQ(ready_queue[cpuID], increment_priority, NULL);
		//addokbuf("Ready Queue non vuota: CARICO pRunning[cpuID]O\n");
		process_count[cpuID]++;

		pRunning[cpuID]->p_s.status |= STATUS_TE;
		setTIMER(5000); /*5ms*/

		LDST(&(pRunning[cpuID]->p_s));
	}
	else{
		if(process_count[cpuID] && !softBlock_count[cpuID])
			PANIC(); /*Deadlock detection*/

		if(process_count && softBlock_count[cpuID]){
			//addokbuf("Ready Queue vuota: Wait\n");
			WAIT();
			while(1);
		}

		if(!process_count[cpuID]){
			addokbuf("Spengo\n");
			HALT();
		}		
	}
}

void pota(){
	while(1);
}


void init(){
	int i;
			
	int cpuID = getPRID();
	for (i = 0; i < MAX_CPUS; i++) {
		addokbuf("Accendo CPU\n");
		scheduler[i].status &= ~(STATUS_IEc|STATUS_KUc| STATUS_VMc);
		scheduler[i].pc_epc = scheduler[i].reg_t9 = (memaddr) schedule;
		scheduler[i].reg_sp = RAMTOP - (FRAME_SIZE * i);
		process_count[cpuID]++;


		/*if(i > 0){
			INITCPU(i,&scheduler[i],new_old_areas[i]);
		}*/
	}

	/* Extracting the first free pcb */
	while(!CAS(&pcb_Lock, 1, 0)) ;
	pcb_t* init_process = allocPcb();
	CAS(&pcb_Lock, 0, 1);

	init_process->p_s.status |= STATUS_IEp|STATUS_INT_UNMASKED;
	init_process->p_s.status &= ~STATUS_KUp;
	init_process->p_s.reg_sp = scheduler[MAX_CPUS-1].reg_sp - FRAME_SIZE;
	init_process->p_s.pc_epc = init_process->p_s.reg_t9 = (memaddr)test; /*p2test*/


	insertProcQ(&ready_queue[cpuID], init_process);
	schedule();
}

void increment_priority(struct pcb_t *pcb)
{
	pcb->priority++; 
}




