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

#define	MAX_CPUS 2

extern void addokbuf(char *strp);
extern pcb_t *current_process;
extern pcb_t *ready_queue[MAX_CPUS];
extern int softBlock_count[MAX_CPUS];
extern state_t *new_old_areas[MAX_CPUS][8];	

state_t scheduler[MAX_CPUS];

// Conta quanti processi nella coda ready della CPU
extern int process_count[MAX_CPUS];

void schedule(){

	int cpuID = getPRID();
	pcb_t *process;
	addokbuf("SCHEDULER\n");

	if((process = removeProcQ(&ready_queue[cpuID])) != NULL){
		forallProcQ(ready_queue[cpuID], increment_priority, NULL);
		addokbuf("Ready Queue non vuota: CARICO PROCESSO\n");
		current_process = process;
		process_count[cpuID]++;

		LDST(&(process->p_s));
	}
	else{
		addokbuf("Ready Queue vuota: CHECK SCHEDULER\n");
		softBlock_count[cpuID]++;

		if(!process_count[cpuID])
			HALT();
		if(process_count[cpuID] && !softBlock_count[cpuID])
			PANIC(); /*Deadlock detection*/
		if(process_count[cpuID] && softBlock_count[cpuID]){
			addokbuf("Ready Queue vuota: Wait\n");
			WAIT();	
		}
	}
}

void pota(){
while(1)
	addokbuf("Pota\n");
}


void init(){
	int i;
	for (i = 1; i < MAX_CPUS; i++) {
		addokbuf("Accendo CPU\n");
		scheduler[i].status &= ~(STATUS_IEc|STATUS_KUc| STATUS_VMc);
		scheduler[i].status |= STATUS_TE;
		scheduler[i].pc_epc = scheduler[i].reg_t9 = (memaddr) schedule;
		scheduler[i].reg_sp = RAMTOP - (FRAME_SIZE * i  );
		process_count[i]++;
		INITCPU(i,&scheduler[i],new_old_areas[i]);
	}
	schedule();
}

void increment_priority(struct pcb_t *pcb)
{
	pcb->priority++; 
}

