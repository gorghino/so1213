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


#include "libumps.h"
#include "const13.h"
#include "pcb.e"
#include "scheduler.h"
#include "main.h"
#include "utils.h"


/* Inizializzo lo scheduler e inserisco l'entrypoint (test) nella readyQueue.
	Le altre CPU vengono attivate quando un processo viene inserito nella loro readyQueue per la prima volta*/
void init(){
	int cpu_index;
	pcb_t* init_process;
	
	/*cpuID contiene l'id del processore in uso - In questo momento solo CPU0*/		
	int cpuID = getPRID();

	/*Preparo gli stati degli scheduler per le altre CPU*/
	for (cpu_index=0; cpu_index < GET_NCPU; cpu_index++) {
		scheduler[cpu_index].status &= ~(STATUS_IEc | STATUS_KUc | STATUS_VMc);
		scheduler[cpu_index].status |= STATUS_TE;
		scheduler[cpu_index].pc_epc = scheduler[cpu_index].reg_t9 = (memaddr) schedule;
		scheduler[cpu_index].reg_sp = RAMTOP - (FRAME_SIZE * cpu_index);
	}

	/* Estraggo il primo PCB dalla lista dei PCBFree */
	init_process = allocPcb();

	/* Associo test() al pcb allocato*/
	init_process->p_s.status = STATUS_INT_UNMASKED|STATUS_TE|STATUS_IEp;
	init_process->p_s.reg_sp = scheduler[GET_NCPU-1].reg_sp - FRAME_SIZE;
	init_process->p_s.pc_epc = init_process->p_s.reg_t9 = (memaddr)test; /*p2test*/

	/*Inserisco il PCB di test() nella readyQueue di CP0*/
	insertProcQ(&ready_queue[cpuID], init_process);
	process_count[cpuID]++;

	/*Attivo il global Interval Timer*/
	SET_IT(SCHED_PSEUDO_CLOCK);
	schedule();
}

/* Lo scheduler() si preoccupa di garantire la finite progress attraverso un meccanismo di round-robin:
	Estrae dalla readyQueue il pcb con priorità massima (situato in testa alla coda), 
	imposta le flag giuste e lo mette in esecuzione con un timeSlice di 4ms.
	Lo scheduler si preoccupa anche di verificare eventuali situazioni di deadlock o di idle.
	Se non vi sono più processi in esecuzioni, lo scheduler spegne la sistema*/
void schedule(){
		int cpuID = getPRID();

		lock(MUTEX_SCHEDULER);
		/* Estraggo il pcb in testa alla readyQueue*/
		if((current_process[cpuID] = removeProcQ(&ready_queue[cpuID])) != NULL){
			unlock(MUTEX_SCHEDULER);
			/* Incremento la priorità dinamica dei pcb */
			forallProcQ(ready_queue[cpuID], increment_priority, NULL);

			/*Preparo il processo da eseguire */
			current_process[cpuID]->p_s.status |= STATUS_TE;
			current_process[cpuID]->priority = current_process[cpuID]->static_priority; /*Ripristino priorità statica*/
			current_process[cpuID]->numCPU = cpuID;

			/*Aggiorno tempo di primo avvio del processo*/
			if(current_process[cpuID]->startTime == 0)
				current_process[cpuID]->startTime = GET_TODLOW;

			setTIMER(SCHED_TIME_SLICE);
			LDST(&(current_process[cpuID]->p_s)); /*Carico il processo sulla CPU appropriata*/
		}
		else{
			unlock(MUTEX_SCHEDULER);
			/* Se la readyQueue è vuota, controllo il nuovo stato della CPU */
			if(process_count[cpuID] && !softBlock_count){
				PANIC(); /*Deadlock!*/
			}

			if( (process_count[cpuID] && softBlock_count) || (!process_count[cpuID] && cpuID)){
				/*In WAIT() finiscono le CPU che hanno processi bloccati o che hanno finito di eseguire i processi a loro assegnati*/
				cpuIdle();
			}

			if(!process_count[cpuID] && !cpuID){
				/*Se CP0 ha finito di eseguire i suoi processi, spengo tutto*/
				HALT();
			}
		}
}

