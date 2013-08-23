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
#include "uMPStypes.h"
#include "asl.e"

 #define	MAX_CPUS 1

extern void addokbuf(char *strp);
extern pcb_t *current_process[MAX_CPUS];
extern pcb_t *ready_queue[MAX_CPUS];
extern int process_count[MAX_CPUS];
extern int softBlock_count[MAX_CPUS];
state_t *sysBp_old = (state_t *)SYSBK_OLDAREA;
state_t *pgmTrap_old = (state_t *)PGMTRAP_OLDAREA;
state_t *tlbTrap_old = (state_t *)TLB_OLDAREA;


void tlbHandler(){
	addokbuf("tlbHandler: Panico!");
	PANIC();
}
void trapHandler(){
	addokbuf("trapHandler: Panico!");
	PANIC();
}

void syscallHandler(){
	int cause = CAUSE_EXCCODE_GET(getCAUSE());
	int cpuID = getPRID();
	pcb_t *unblocked;
	switch(cause){
		case EXC_SYSCALL: 
			addokbuf("SYSCALL\n"); 
			switch(sysBp_old->reg_a0){
				case CREATEPROCESS: 
					addokbuf("CREATEPROCESS\n"); 
					state_t *child_state;
					pcb_t *child;
					//a1 should contain the physical address of a processor state
					//area at the time this instruction is executed.
					//This processor state should be used
					//as the initial state for the newly created process
					child_state = (state_t *)(current_process[cpuID]->p_s).reg_a1;
					//If the new process cannot be created due
					//to lack of resources (for example no more free ProcBlk’s),
					// an error code of -1 is
					//placed/returned in the caller’s v0,
					// otherwise, return the value 0 in the caller’s v0.
					//The SYS1 service is requested by the calling process by placing the value
					//1 in a0, the physical address of a processor state in a1, and then executing a
					//SYSCALL instruction.
					if ( ( child = allocPcb() ) != NULL ) {
						(current_process[cpuID]->p_s).reg_v0 = -1;
					}
					else {
						(current_process[cpuID]->p_s).reg_v0 = 0;	
						child_state=&(child->p_s);
						child->priority=(current_process[cpuID]->p_s).reg_a2;
						insertChild(current_process[cpuID], child);
						insertProcQ(&ready_queue[cpuID], child);
					} 
					break;

				case TERMINATEPROCESS: 
					addokbuf("TERMINATEPROCESS\n"); 
					outChildBlocked(current_process[cpuID]);					
					break;

				case VERHOGEN: 
					addokbuf("VERHOGEN\n"); 
					int *semV = sysBp_old->reg_a1;
					(*semV)++;
					// Se ci sono processi bloccati, il primo viene tolto dalla coda e messo in readyQueue
					if ((unblocked = removeBlocked(sysBp_old->reg_a1)) != NULL){
						softBlock_count[cpuID]--;
						insertProcQ(&ready_queue, unblocked);
					}
					break;

				case PASSEREN: 
					addokbuf("PASSEREN\n"); 
					int *semP = sysBp_old->reg_a1;
					(*semP)--;
					if(*semP < 0){
						// Se il valore del semaforo è negativo, il processo viene bloccato e accodato 
						softBlock_count[cpuID]++;
						insertBlocked(semP, current_process);
					}
					break;

				case SPECTRAPVEC: addokbuf("SPECTRAPVEC\n"); break;
				case GETCPUTIME: addokbuf("GETCPUTIME\n"); break;
				case WAITCLOCK: addokbuf("WAITCLOCK\n"); break;

				/*int SYSCALL(WAITIO, int intNo, int dnum, int waitForTermRead)	
  					Quando invocata, la SYS8 esegue una P sul semaforo associato al device idenficato da intNo, dnume e waitForTermRead*/
				case WAITIO: addokbuf("WAITIO\n"); break;
			}
		break;
		case EXC_BREAKPOINT: addokbuf("BREAKPOINT\n"); break;
	}
	sysBp_old->pc_epc += 4; 
	LDST(sysBp_old); 
}
