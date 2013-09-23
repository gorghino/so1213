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
#include "const13_customized.h"
#include "base.h"
#include "uMPStypes.h"
#include "types13.h"
#include "asl.e"
#include "utils.h"
#include "main.h"
#include "scheduler.h"

/*insertPCBList inserisce il pcb puntato da pcb_elem nella lista puntata da pcblist_p*/
void insertPCBList(pcb_t **pcblist_p, pcb_t *pcb_elem){
	if(*pcblist_p == NULL){ 
		/*Caso empty*/
		*pcblist_p = pcb_elem;
		pcb_elem->p_next = NULL;
		return;
	}
	else if ((*pcblist_p)->p_next == NULL) { 
		/*Inserimento*/
		(*pcblist_p)->p_next = pcb_elem;
		pcb_elem->p_next = NULL;
	}
	else insertPCBList(&((*pcblist_p)->p_next), pcb_elem);
}

/*insertSEMList inserisce il semaforo puntato da sem_elem nella lista puntata da semlist_p*/
void insertSEMList(semd_t **semlist_p, semd_t *sem_elem){
	if(*semlist_p == NULL){
		/*Caso empty*/
		*semlist_p = sem_elem;
		sem_elem->s_next = NULL;
		return;
	}
	else if ((*semlist_p)->s_next == NULL){
		/*Inserimento*/
		(*semlist_p)->s_next = sem_elem;
		sem_elem->s_next = NULL;
	}
	else insertSEMList(&(*semlist_p)->s_next, sem_elem);
}

/*Controlla se la lista di pcb_t* puntata da pcblist_p è vuota*/
int emptyPCBList(pcb_t **pcblist_p){
	if(*pcblist_p == NULL)
		return TRUE;
	else
		return FALSE;
}

/*Controlla se la lista di semd_t* puntata da semlist_p è vuota*/
int emptySEMList(semd_t **semlist_p){
	if(*semlist_p == NULL)
		return TRUE;
	else
		return FALSE;
}

/*insertSibling aggiunge il puntatore al pcb P appena trova un pcb figlio senza fratello nella lista puntata da firstchild*/
void insertSibling(pcb_t *firstchild, pcb_t *p){
	if (firstchild->p_sib == NULL){
		firstchild->p_sib = p;

		p->p_parent = firstchild->p_parent; /*Ogni figlio punta al padre*/
		p->p_sib = NULL;
		}
	else insertSibling(firstchild->p_sib, p);
}

/*P() verifica se il semaforo con chiave key esista; se esiste, ne decrementa il valore e se necessario, blocca il pcb process
nella coda del semaforo.
Se non viene inizialmente trovato un semaforo, ne viene allocato uno nuovo con chiave key*/
int P(int *key, pcb_t *process){
	/*Cerco il semaforo con chiave key*/
	semd_t *semd = getSemd(key);
	lock(MUTEX_PV);
	if(semd !=NULL){
		/*Il semaforo esiste ed è nella ASL*/
		(*semd->s_key)--;
		if((*semd->s_key) < 0){
			/*Se il valore del semaforo è negativo, accodo il processo chiamante*/
			insertBlocked(key, process);
			plusSoftCounter();
			unlock(MUTEX_PV);
			return TRUE; /*Mi blocco*/
		}
		else{
			unlock(MUTEX_PV);
			return FALSE; /*Non mi blocco*/
		}
	}
	else{
		/*Il semaforo non esiste nella ASL*/
		(*key)--;
		if((*key) < 0){
			insertBlocked(key, process); /* Alloca il semaforo se non esiste*/
			unlock(MUTEX_PV);
			plusSoftCounter();
			unlock(MUTEX_PV);
			return TRUE; /*Mi blocco*/
		}
		unlock(MUTEX_PV);
		return FALSE; /*Non mi blocco*/
	}
}

/*V() verifica se il semaforo con chiave key esista; se esiste, ne incrementa il valore e se necessario, sblocca il pcb in testa
alla coda del semaforo.
Se non viene inizialmente trovato un semaforo, ne incremento comunque il valore. Alla sua allocazione futura avrà quindi
un valore diverso da quello di default*/
pcb_t* V(int* key){	
	semd_t *semd;
	pcb_t *unblocked;
	lock(MUTEX_PV);
	/*Cerco il semaforo con chiave key*/
	if((semd = getSemd(key))!=NULL){
		/*Il semaforo esiste ed è nella ASL*/
		(*semd->s_key)++;		
		if((*semd->s_key) >= 0){
			/*Sblocco il processo*/
			unblocked = removeBlocked(key);
			lessSoftCounter();
			unlock(MUTEX_PV);
			return unblocked;
		}
	}
	else
		/*Il semaforo non esiste nella ASL*/
		(*key)++;

	unlock(MUTEX_PV);
	return NULL;

}

/*findDeviceNumber() viene utilizzato per identificare il numero del device che ha sollevato l'interrupt*/
int findDeviceNumber(memaddr* bitmap) {
  int device_n = 0;
  
  while (*bitmap > 1) {
    device_n++;
    *bitmap >>= 1;
  }
  return device_n;
}

/*lock() e unlock() sono utilizzate per la mutua esclusione, sfruttando la funzione CAS di uMPS*/
void lock(int semkey){
	while (!CAS(&semArray[semkey],1,0)) ;
}
void unlock(int semkey){
	CAS(&semArray[semkey],0,1);
}

/*initNewOldArea() è utilizzato in fase di boot per inizializzare l'area passata come primo argomento*/
void initNewOldArea(state_t * area, memaddr handler, int offset){
	area->pc_epc = area->reg_t9 = handler; /*Imposto il program counter sulla funzione gestore*/
	area->reg_sp = RAMTOP - (FRAME_SIZE * offset); /*Imposto l'indirizzo di memoria adatto*/
	area->status &= ~(STATUS_IEc|STATUS_KUc|STATUS_VMc); /*Imposto i flag adatti*/
	area->status |= STATUS_TE;
}

/*increment_priority() è utilizzato dallo scheduler per incrementare la priorità dinamica dei processi rimasti in readyQueue*/
void increment_priority(struct pcb_t *pcb, void* pt){
	if(pcb->priority != 19)
		pcb->priority++;
	return;
}

/*cpuIdle() è utilizzato dallo scheduler per impostare lo stato della CPU in stato WAIT*/
void cpuIdle(){
	int status = getSTATUS() | STATUS_IEc | STATUS_INT_UNMASKED;
	setSTATUS(status);
	while(1) WAIT();	
}

/*killMe() termina il processo chiamante sulla cpu con id cpuID e restituisce il controllo allo scheduler*/
void killMe(int cpuID){
	outChildBlocked(current_process[cpuID]);
	current_process[cpuID] = NULL;
	LDST(&scheduler[cpuID]);
}

/*isUserMode() verifica se il processo in esecuzione su cpuID è in stato USER*/
int isUserMode(int cpuID){
	return ((current_process[cpuID]->p_s.status << 28) >> 31);
}

/*plusSoftCounter() incrementa il softBlock in mutua esclusione*/
void plusSoftCounter() {
	while (!CAS(&semArray[MUTEX_SOFTBLOCK],1,0)) ;
	softBlock_count++;
	CAS(&semArray[MUTEX_SOFTBLOCK],0,1);
}

/*lessSoftCounter() decrementa il softBlock in mutua esclusione*/
void lessSoftCounter() {
	while (!CAS(&semArray[MUTEX_SOFTBLOCK],1,0)) ;
	softBlock_count--;
	CAS(&semArray[MUTEX_SOFTBLOCK],0,1);
}

/*checkSemaphore() verifica se p è bloccato su un semaforo di un device*/
int checkSemaphore(pcb_t *p){
	int j;
	int * key = p->p_semkey;
	for(j=0; j<DEV_PER_INT; j++)
		if(key == &sem_disk[j] || \
			key == &sem_tape[j] || \
				key == &sem_ethernet[j] || \
					key == &sem_printer[j] || \
						key == &sem_terminal_read[j] || \
							key == &sem_terminal_write[j])
			return TRUE;
	return FALSE;
}

