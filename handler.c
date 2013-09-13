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


/*definizione per states_array[6]*/ 
 
 #define TLB_OLD 		0
 #define TLB_NEW		1
 #define PGMTRAP_OLD 	2
 #define PGMTRAP_NEW 3
 #define SYSBREAKPOINT_OLD 	4
 #define SYSBREAKPOINT_NEW 	5	

#include "libumps.h"
#include "const13.h"
#include "uMPStypes.h"
#include "asl.e"
#include "handler.h"
#include "utils.h"
#include "print.h"
#include "main.h"
#include "scheduler.h"
#include "const13_customized.h"

 #define	MAX_CPUS 1

extern void addokbuf(char *strp);
extern pcb_t *current_process[MAX_CPUS];
extern pcb_t *ready_queue[MAX_CPUS];
extern int process_count[MAX_CPUS];
extern int softBlock_count[MAX_CPUS];
state_t *sysBp_old = (state_t *)SYSBK_OLDAREA;
state_t *pgmTrap_old = (state_t *)PGMTRAP_OLDAREA;
state_t *tlbTrap_old = (state_t *)TLB_OLDAREA;
state_t *sysBp_new = (state_t *)SYSBK_NEWAREA;


void tlbHandler(){
	addokbuf("tlbHandler: Panico!");
	PANIC();
}
void trapHandler(){
	addokbuf("trapHandler: Panico!");
	PANIC();
}

int*semVV;

void syscallHandler(){
	int cause = CAUSE_EXCCODE_GET(getCAUSE());
	int cpuID = getPRID();
	pcb_t *unblocked;
	pcb_t *child;
	state_t *child_state;
	int *semV = (int*) sysBp_old->reg_a1;
	int *semP = (int *) sysBp_old->reg_a1;

	copyState(((state_t*)SYSBK_OLDAREA), &(current_process[cpuID]->p_s));

	char buffer[1024];
	switch(cause){
		case EXC_SYSCALL: 	
			//addokbuf("SYSCALL\n"); 
			switch(current_process[cpuID]->p_s.reg_a0){
				case CREATEPROCESS: 

					/*The SYS1 service is requested by the calling process by placing the value
					1 in a0, the physical address of a processor state in a1, and then executing a
					SYSCALL instruction.*/

					/*a1 should contain the physical address of a processor state
					area at the time this instruction is executed.
					This processor state should be used
					as the initial state for the newly created process*/
					child_state = (state_t *)current_process[cpuID]->p_s.reg_a1;

					/*If the new process cannot be created due to lack of resources (for example no more free ProcBlk’s),
					 an error code of -1 is placed/returned in the caller’s v0, otherwise, 
					 return the value 0 in the caller’s v0.
					*/
					if (( child = allocPcb() ) == NULL)
						(current_process[cpuID]->p_s).reg_v0 = -1;
					else {
						copyState(child_state, &(child->p_s));
						child->priority=(current_process[cpuID]->p_s).reg_a2;
						insertChild(current_process[cpuID], child);
						insertProcQ(&ready_queue[cpuID], child);
						(current_process[cpuID]->p_s).reg_v0 = 0;
					}

					current_process[cpuID]->p_s.pc_epc += 4;
					break;

				case TERMINATEPROCESS:
					pota_debug2();
					//addokbuf("TERMINATEPROCESS\n"); 
					outChildBlocked(current_process[cpuID]);
					pota_debug2();			
					break;

				case VERHOGEN:

					//addokbuf("VERHOGEN\n"); 
					(*semV)++;
					/* Se ci sono processi bloccati, il primo viene tolto dalla coda e messo in readyQueue*/
					if ((unblocked = removeBlocked(semV)) != NULL){
						softBlock_count[cpuID]--;
						insertProcQ(&ready_queue[cpuID], unblocked);
					}
					current_process[cpuID]->p_s.pc_epc += 4;
					break;

				case PASSEREN:
					//addokbuf("PASSEREN\n"); 
					if((*semP) >= 0)
						(*semP)--;
					if(*semP < 0){
						/*Se il valore del semaforo è negativo, il processo viene bloccato e accodato */
						softBlock_count[cpuID]++;
						current_process[cpuID]->p_s.pc_epc += 4;
						insertBlocked(semP, current_process[cpuID]);
						pota_debug();
						LDST(&scheduler[cpuID]);
					}
					else
						current_process[cpuID]->p_s.pc_epc += 4;
					break;

				case SPECTRAPVEC: 
					//addokbuf("SPECTRAPVEC\n"); 
					/*nel registro a1 ho il tipo di eccezione e da li ho 3 casi. (TLB exceptions, PGMTRAP e SysBp).
					Nella struttura pcb abbiamo un array [0-5] dove vengono salvati gli stati del processore.
					Nei reg_a3 abbiamo i NEW (da utilizzare nel caso si verifichino exceptions o PGMTRAP) 
					e nei reg_a2 gli OLD, salviamo anche questi nel nostro states_array[6]*/

					
					if(!current_process[cpuID]->p_s.reg_a1){
						/*TLB exceptions*/
					current_process[cpuID]->states_array[TLB_NEW] = (state_t*) current_process[cpuID]->p_s.reg_a3;
					current_process[cpuID]->states_array[TLB_OLD] = (state_t*) current_process[cpuID]->p_s.reg_a2;
						}
					else if(current_process[cpuID]->p_s.reg_a1==1){
						/*PGMTRAP exceptions*/
						current_process[cpuID]->states_array[PGMTRAP_NEW] = (state_t*) current_process[cpuID]->p_s.reg_a3;
						current_process[cpuID]->states_array[PGMTRAP_OLD] = (state_t*) current_process[cpuID]->p_s.reg_a2;
					}
					else if (current_process[cpuID]->p_s.reg_a1==2){
						/*SysBp exceptions*/
						current_process[cpuID]->states_array[SYSBREAKPOINT_NEW] = (state_t*) current_process[cpuID]->p_s.reg_a3;
						current_process[cpuID]->states_array[SYSBREAKPOINT_OLD] = (state_t*) current_process[cpuID]->p_s.reg_a2;
					}
					break;
                    
				case GETCPUTIME: 
					//addokbuf("GETCPUTIME\n"); 
					current_process[cpuID]->p_s.reg_v0 = (GET_TODLOW - current_process[cpuID]->startTime);
					current_process[cpuID]->p_s.pc_epc += 4;
					break;
					
				case WAITCLOCK: 
					//addokbuf("WAITCLOCK\n"); 
					P((int*) SCHED_PSEUDO_CLOCK, current_process[cpuID]);
					current_process[cpuID]->p_s.pc_epc += 4;
					break;

				/*int SYSCALL(WAITIO, int intNo, int dnum, int waitForTermRead)	
  					Quando invocata, la SYS8 esegue una P sul semaforo associato al device idenficato da intNo, dnume e waitForTermRead*/
				case WAITIO: 
					//addokbuf("WAITIO\n"); 
					/*verificare se si è in attesa di I/O*/
					if (current_process[cpuID]->p_s.reg_a1 <= INT_TERMINAL) {
					
						int devNumber = current_process[cpuID]->p_s.reg_a2;
						//itoa(devNumber, buffer, 10);
						//addokbuf(buffer);		
						current_process[cpuID]->p_s.pc_epc += 4;

						if(!(current_process[cpuID]->p_s.reg_a3)){
							//Caso write
							if(P(&sem_terminal_read[devNumber], current_process[cpuID])){
								current_process[cpuID] = NULL;
								LDST(&scheduler[cpuID]);
							}
							else{
								current_process[cpuID]->p_s.reg_v0 = device_write_response[(current_process[cpuID]->p_s.reg_a2)];		
							}
						}					
						else{
							//Caso read
							if(P(&sem_terminal_write[devNumber], current_process[cpuID])){
								LDST(&scheduler[cpuID]);
							}
							else
								current_process[cpuID]->p_s.reg_v0 = device_read_response[current_process[cpuID]->p_s.reg_a2];			
						}
						insertProcQ(&ready_queue[cpuID], current_process[cpuID]);
					}
					break;
			} 
			break;	
		case EXC_BREAKPOINT:
			addokbuf("z");
			/*addokbuf("BREAKPOINT\n");*/
			break;
	}
	LDST(&current_process[cpuID]->p_s); 
}
