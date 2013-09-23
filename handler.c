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
#include "handler.h"
#include "utils.h"
#include "scheduler.h"
#include "const13_customized.h"
#include "main.h"

/* Gestore dei TLB-EXCEPTION.
	Una eccezione TLB avviene quando μMPS2 fallisce nel tentativo di tradurre un indirizzo virtuale in fisico*/
void tlbHandler(){
	int cpuID = getPRID();
	/*Se il processo coinvolto ha precedentemente specificato il proprio gestore (SYS5),
		allora l'eccezione viene "passed-up" a un livello superiore del kernel, copiando le aree corrispondenti,
		altrimenti viene terminato*/
	if(current_process[cpuID]->states_array[TLB_NEW] != NULL){
		cpuID > 0 ? copyState(&new_old_areas[cpuID][TLB_OLDAREA_INDEX], (current_process[cpuID]->states_array[TLB_OLD])) : \
					copyState(((state_t*)TLB_OLDAREA), (current_process[cpuID]->states_array[TLB_OLD]));

		copyState((current_process[cpuID]->states_array[TLB_NEW]), &current_process[cpuID]->p_s);
		LDST(&current_process[cpuID]->p_s);
	}
	else 
		killMe(cpuID); /* Il processo corrente viene terminato e il controllo tornato allo scheduler*/
}

/* Gestore dei PgmTRAP-EXCEPTION.
	Una eccezione PgmTRAP avviene quando il processo in esecuzione tenta un'operazione illegale (Vedi Section 3.1.1-pops)*/
void trapHandler(){
	int cpuID = getPRID();
	/*se il processo che ha chiamato la system call, ha gia chiamato la sys5 per un PGM TRAP,
	lo stato del processore viene copiato dalla PGMTRAP old area nell'area dello stato del processore apposito*/
	if(current_process[cpuID]->states_array[PGMTRAP_NEW] != NULL){
		cpuID > 0 ? copyState(&new_old_areas[cpuID][PGMTRAP_OLDAREA_INDEX], (current_process[cpuID]->states_array[PGMTRAP_OLD])) : \
					copyState(((state_t*)PGMTRAP_OLDAREA), (current_process[cpuID]->states_array[PGMTRAP_OLD]));

		copyState((current_process[cpuID]->states_array[PGMTRAP_NEW]), &current_process[cpuID]->p_s);
		LDST(&current_process[cpuID]->p_s); 
	}
	else
		killMe(cpuID); /* Il processo corrente viene terminato e il controllo tornato allo scheduler*/	
}

/* Gestore delle Syscall
	Una Syscall o Breakpoint avviene quando viene esplicitamente chiamata l'istruzione SYSCALL() o BREAK()*/
void syscallHandler(){
	int cpuID = getPRID();
	pcb_t *unblocked;
	pcb_t *child;
	state_t *child_state;
	int *semV, *semP;

	/*Verifico se è chiamato SYSCALL() o BREAK()*/
	int cause = CAUSE_EXCCODE_GET(getCAUSE());

	cpuID > 0 ? copyState(&new_old_areas[cpuID][SYSBK_OLDAREA_INDEX], &(current_process[cpuID]->p_s)) : \
				copyState(((state_t*)SYSBK_OLDAREA), &(current_process[cpuID]->p_s));

	/* Check User/Kernel mode. Controllo se una istruzione privilegiata è stata chiamata in user-mode, 
		in tal caso chiamo il trapHandler*/
	if(current_process[cpuID]->p_s.reg_a0 <= SYSCALL_MAX && isUserMode(cpuID) ){
		if(cpuID > 0){
			copyState(&new_old_areas[cpuID][SYSBK_OLDAREA_INDEX], &new_old_areas[cpuID][PGMTRAP_OLDAREA_INDEX]);
			new_old_areas[cpuID][PGMTRAP_OLDAREA_INDEX].cause = CAUSE_EXCCODE_SET(CAUSE_EXCCODE_GET(new_old_areas[cpuID][PGMTRAP_OLDAREA_INDEX].cause), EXC_RESERVEDINSTR);
		}
		else{
			copyState((state_t*)SYSBK_OLDAREA, (state_t *)PGMTRAP_OLDAREA);
			((state_t *)PGMTRAP_OLDAREA)->cause = CAUSE_EXCCODE_SET(CAUSE_EXCCODE_GET( ((state_t *)PGMTRAP_OLDAREA)->cause), EXC_RESERVEDINSTR);
		}
		trapHandler();
	}

	switch(cause){
		case EXC_SYSCALL: 	
			switch(current_process[cpuID]->p_s.reg_a0){
				case CREATEPROCESS: 
					/*La CREATEPROCESS (SYS1) crea un processo figlio del processo chiamante.
					Nel registro a1 abbiamo lo stato del processo figlio,
						nel registro a2 la priorità e
						nel registro a3 il numero di processore sul quale caricarlo*/
					child_state = (state_t *)current_process[cpuID]->p_s.reg_a1;
					int newCPU = (current_process[cpuID]->p_s.reg_a3 % GET_NCPU);

					if (( child = allocPcb() ) == NULL){
						/*Se la PcbTable è piena, torno -1*/
						(current_process[newCPU]->p_s).reg_v0 = -1;
					}
					else {
						/* Un pcb è stato allocato e viene quindi inizializzato*/
						copyState(child_state, &(child->p_s));
						child->static_priority = child->priority = (current_process[cpuID]->p_s).reg_a2;
						child->numCPU = newCPU;
						insertChild(current_process[cpuID], child);

						lock(MUTEX_SCHEDULER);
						insertProcQ(&ready_queue[newCPU], child);
						process_count[newCPU]++;
						unlock(MUTEX_SCHEDULER);

						/*Se il sistema prevede Multi Processori, allora inizializzo il processo nell'apposita readyQueue*/
						if(stateCPU[newCPU] == STOPPED && newCPU > 0 && GET_NCPU != 1){
							stateCPU[newCPU] = RUNNING;
							INITCPU(newCPU,&scheduler[newCPU],&new_old_areas[newCPU]);
						}
						/*Ritorno OK: 0*/
						(current_process[cpuID]->p_s).reg_v0 = 0;
					}
					
					current_process[cpuID]->p_s.pc_epc += WORD_SIZE;
					break;

				case TERMINATEPROCESS:
					/*La TERMINATEPROCESS (SYS2) termina il processo chiamante e la sua progenie*/
					killMe(cpuID);
					break;

				case VERHOGEN:
					/*La VERHOGEN (SYS3) incrementa il semaforo specificato nel primo parametro (registro a1)*/
					semV = (int*) current_process[cpuID]->p_s.reg_a1;
					lock(MUTEX_PV);
					(*semV)++;
					/* Se ci sono processi bloccati, il primo viene tolto dalla coda e messo in readyQueue*/
					if(*semV <= 0)
						if ((unblocked = removeBlocked(semV)) != NULL){
							lessSoftCounter(); /*Decremento il softBlock counter*/
							lock(MUTEX_SCHEDULER);
							/*Inserisco il processo sbloccato nella sua readyQueue*/
							insertProcQ(&ready_queue[unblocked->numCPU], unblocked);
							unlock(MUTEX_SCHEDULER);
						}
					current_process[cpuID]->p_s.pc_epc += WORD_SIZE;
					unlock(MUTEX_PV);
					break;

				case PASSEREN:
					/*La PASSEREN (SYS4) decrementa il semaforo specificato nel primo parametro (registro a1)
					e, nel caso, blocca il processo chiamante*/
					semP = (int*) current_process[cpuID]->p_s.reg_a1;
					lock(MUTEX_PV);
					(*semP)--;
					if(*semP < 0){
						/*Se il valore del semaforo è negativo, il processo viene bloccato e accodato */
						current_process[cpuID]->p_s.pc_epc += WORD_SIZE;
						insertBlocked(semP, current_process[cpuID]);
						plusSoftCounter(); /*Incremento il softBlock counter*/
						current_process[cpuID] = NULL;
						unlock(MUTEX_PV);
						LDST(&scheduler[cpuID]);
					}
					else{
						/*Il chiamante non si blocca*/
						current_process[cpuID]->p_s.pc_epc += WORD_SIZE;
						unlock(MUTEX_PV);
					}
					break;

				case SPECTRAPVEC:
					/*La SPECTRAPVEC (SYS5) permette di specificare un gestore di trap personale
					Nella funzione vengono quindi memorizzate in un'apposita struttura in pcb_t,
					le nuove new_old_areas.
					Nel registro a1 viene specificato il tipo di eccezione (TLB exceptions, PGMTRAP e SysBp).
					Nel registro a3 abbiamo la NEW_AREA (da utilizzare nel caso si verifichino exceptions o PGMTRAP) 
					e nel registro a2 la OLD_AREA*/

					current_process[cpuID]->p_s.pc_epc += WORD_SIZE;
					
					switch(current_process[cpuID]->p_s.reg_a1){
						case 0:
							/*TLB exceptions*/	
							if(!current_process[cpuID]->states_array[TLB_OLD]){
								current_process[cpuID]->states_array[TLB_OLD] = (state_t*) current_process[cpuID]->p_s.reg_a2;
								current_process[cpuID]->states_array[TLB_NEW] = (state_t*) current_process[cpuID]->p_s.reg_a3;
							}
							else
								/*TLB exceptions già impostata. Killo*/
								killMe(cpuID);	
							break;
						case 1:
							/*PGMTRAP exceptions*/
							if(!current_process[cpuID]->states_array[PGMTRAP_OLD]){
								current_process[cpuID]->states_array[PGMTRAP_OLD] = (state_t*) current_process[cpuID]->p_s.reg_a2;
								current_process[cpuID]->states_array[PGMTRAP_NEW] = (state_t*) current_process[cpuID]->p_s.reg_a3;
							}
							else
								/*PgmTrap exceptions già impostata. Killo*/
								killMe(cpuID);	
							break;
						case 2:
							/*SysBp exceptions*/
							if(!current_process[cpuID]->states_array[SYSBK_OLD]){
								current_process[cpuID]->states_array[SYSBK_OLD] = (state_t*) current_process[cpuID]->p_s.reg_a2;
								current_process[cpuID]->states_array[SYSBK_NEW] = (state_t*) current_process[cpuID]->p_s.reg_a3;
							}
							else
								/*SysBp exceptions già impostata. Killo*/
								killMe(cpuID);	
							break;
					}
					break;
                    
				case GETCPUTIME: 
					/*La GETCPUTIME (SYS6) ritorna al chiamante il tempo CPU utilizzato dal processo corrente,
					sottraendo all'istante in cui viene invocato, l'istante di partenza del processo */
					current_process[cpuID]->p_s.reg_v0 = (GET_TODLOW - current_process[cpuID]->startTime);
					current_process[cpuID]->p_s.pc_epc += WORD_SIZE;
					break;
					
				case WAITCLOCK:
					/*La WAITCLOCK (SYS7) blocca il chiamante sullo PseudoClock in attesa del timer*/
					current_process[cpuID]->p_s.pc_epc += WORD_SIZE;
					lock(MUTEX_CLOCK);
					if(P(&pseudo_clock, current_process[cpuID])){
						current_process[cpuID] = NULL;
						unlock(MUTEX_CLOCK);
						LDST(&scheduler[cpuID]);
					}
					unlock(MUTEX_CLOCK);
					break;
			
  					
				case WAITIO:
					/*La WAITIO (SYS8) permette al chiamante di aspettare un INTERRUPT da un device, bloccandosi,
					se necessario, sul relativo semaforo.
					Il device è idenficato da intNo, dnume e waitForTermRead*/
					/*Verifico se siamo in attesa di I/O*/ 
					switch(current_process[cpuID]->p_s.reg_a1){
						case INT_TERMINAL: {
							int devNumber = current_process[cpuID]->p_s.reg_a2;		
							current_process[cpuID]->p_s.pc_epc += WORD_SIZE;
							
							/*Il registro a3 (boolean) indica I/O:
								1: Caso read
								0: Caso Write*/
							if(!(current_process[cpuID]->p_s.reg_a3)){
								/*Caso write*/
								if(P(&sem_terminal_write[devNumber], current_process[cpuID])){
									/*Avviene prima la syscall. Il processo si blocca sul semaforo in attesa dell'INTERRUPT*/
									current_process[cpuID] = NULL;
									LDST(&scheduler[cpuID]);
								}
								else{
									/*Avviene prima l'interrupt. Il processo NON si blocca sul semaforo e trova lo status 
										nella struttura globale del terminale*/
									current_process[cpuID]->p_s.reg_v0 = device_write_response[(current_process[cpuID]->p_s.reg_a2)];
									device_write_response[(current_process[cpuID]->p_s.reg_a2)] = 0;
								}
							}					
							else{
								/*Caso read*/
								if(P(&sem_terminal_read[devNumber], current_process[cpuID])){
									/*Avviene prima la syscall. Il processo si blocca sul semaforo in attesa dell'INTERRUPT*/
									current_process[cpuID] = NULL;
									LDST(&scheduler[cpuID]);
								}
								else{
									/*Avviene prima l'interrupt. Il processo NON si blocca sul semaforo e trova lo status 
										nella struttura globale del terminale*/
									current_process[cpuID]->p_s.reg_v0 = device_read_response[current_process[cpuID]->p_s.reg_a2];
									device_read_response[(current_process[cpuID]->p_s.reg_a2)] = 0;			
								}
							}
							break;
						}
						case INT_DISK: {
							int devNumber = current_process[cpuID]->p_s.reg_a2;		
							current_process[cpuID]->p_s.pc_epc += WORD_SIZE;
							if(P(&sem_terminal_write[devNumber], current_process[cpuID])){
								/*Avviene prima la syscall. Il processo si blocca sul semaforo in attesa dell'INTERRUPT*/
								current_process[cpuID] = NULL;
								LDST(&scheduler[cpuID]);
							}
							break;
						}
						case INT_TAPE : {
							int devNumber = current_process[cpuID]->p_s.reg_a2;		
							current_process[cpuID]->p_s.pc_epc += WORD_SIZE;
							if(P(&sem_terminal_write[devNumber], current_process[cpuID])){
								/*Avviene prima la syscall. Il processo si blocca sul semaforo in attesa dell'INTERRUPT*/
								current_process[cpuID] = NULL;
								LDST(&scheduler[cpuID]);
							}
							break;
						}
						case INT_UNUSED : {
							int devNumber = current_process[cpuID]->p_s.reg_a2;		
							current_process[cpuID]->p_s.pc_epc += WORD_SIZE;
							if(P(&sem_terminal_write[devNumber], current_process[cpuID])){
								/*Avviene prima la syscall. Il processo si blocca sul semaforo in attesa dell'INTERRUPT*/
								current_process[cpuID] = NULL;
								LDST(&scheduler[cpuID]);
							}
							break;
						}
						case INT_PRINTER : {
							int devNumber = current_process[cpuID]->p_s.reg_a2;		
							current_process[cpuID]->p_s.pc_epc += WORD_SIZE;
							if(P(&sem_terminal_write[devNumber], current_process[cpuID])){
								/*Avviene prima la syscall. Il processo si blocca sul semaforo in attesa dell'INTERRUPT*/
								current_process[cpuID] = NULL;
								LDST(&scheduler[cpuID]);
							}
							break;
						}
					}

					break;

				default:
					/*Viene chiamata una SYSCALL fuori range. Controllo se è stato specificato SYS5*/
					if(current_process[cpuID]->states_array[SYSBK_NEW] != NULL){
						cpuID > 0 ? copyState(&new_old_areas[cpuID][SYSBK_OLDAREA_INDEX], (current_process[cpuID]->states_array[SYSBK_OLD])) : \
									copyState(((state_t*)SYSBK_OLDAREA), (current_process[cpuID]->states_array[SYSBK_OLD]));
						
						copyState((current_process[cpuID]->states_array[SYSBK_NEW]), &current_process[cpuID]->p_s);
					}
					else
						/*Non è stata chiamata SYS5. Killo*/
						killMe();

					break;
			} 
			break;	

		case EXC_BREAKPOINT:
			/* Non gestito */
			HALT();
			break;
	}
	/* Ricarico il processo chiamante con PC aumentati di una WORD*/
	LDST(&current_process[cpuID]->p_s); 
}

