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


#include "asl.e"
#include "utils.h"
#include "p1test.h"

semd_t semd_table[MAXPROC]; /*Tabella dei semafori*/
semd_t *semdFree_h; /*Puntatore alla testa della lista dei semafori liberi*/
semd_t *semd_h; /*Puntatore alla testa della lista dei semafori attivi (ASL)*/

#define TRUE 1
#define FALSE 0


/************ Funzioni per gestire le liste di SEMafori ************/

/*Inizializza la lista dei semdFree in modo da contenere tutti gli elementi della semdTable. 
Questo metodo viene invocato una volta sola durante l’inizializzazione della strutuura dati.
N.b. Sebbene il nome sia fuorviante, initASL NON inizializza la lista dei semafori attivi, ma quella dei semafori liberi*/
void initASL(){
	initASL_rec(0);
}

void initASL_rec(int count){
	if (count>=MAXPROC) /*0...MAXPROC-1*/
		return;

	insertSEMList(&semdFree_h, &semd_table[count]);
	count++;
	initASL_rec(count);
}

/*Cerca nella ASL il semaforo con chiave key e restituisce un puntatore ad esso*/
semd_t* getSemd(int *key){
	//addokbuf("Cerco semaforo\n");
	return getSemd_rec(&semd_h, key);
}

semd_t* getSemd_rec(semd_t **semd_h, int* key){
	if(*semd_h == NULL){
		//addokbuf("Semaforo non e' nella ASL\n");
		return NULL; /*Il semaforo non esiste nella ASL*/
	}

	if((*semd_h)->s_key == key)
		return *semd_h;

	else 
		return getSemd_rec(&((*semd_h)->s_next), key);
}

/*Viene inserito il PCB puntato da p nella coda dei processi bloccati associata al SEMD con chiave key. 
Se il semaforo corrispondente non è presente nella ASL, alloca un nuovo SEMD dalla lista di quelli liberi (semdFree) e lo inserisce nella ASL.
Se non è possibile allocare un nuovo SEMD perchè la lista di quelli liberi è vuota, restituisce TRUE. 
In tutti gli altri casi,restituisce FALSE*/
int insertBlocked(int *key, pcb_t* p){
		semd_t *semd_target = getSemd(key);
		if(semd_target == NULL){
			/*Il semaforo non esiste nella ASL*/

			semd_target = allocSem(); /*semd_target ora punta al SEMD tolto dalla semdFree. Se semdFree è vuota, semd_target == NULL*/
			if (semd_target == NULL) /*Se la semdFree è vuota restituisco TRUE*/
				return TRUE;
			semd_target->s_key = key; 
			
			insertSEMList(&semd_h, semd_target); /*Alloco semd_target nella ASL*/
			p->p_semkey = key;
			insertProcQ(&(semd_target->s_procQ), p); /*Inserisco p nella coda di processi bloccati di semd_target*/
			return FALSE;
			}
		else{
			/*Il semaforo esiste già nella ASL*/
			insertProcQ(&(semd_target->s_procQ), p);
			return FALSE;
		}	
}

/*Estrae un SEM dalla lista dei semdFree e restituisce un puntatore a esso*/
semd_t *allocSem(){
	if(emptySEMList(&semdFree_h))
		return NULL;
	else{
		semd_t *ptemp = semdFree_h;
		semdFree_h = semdFree_h->s_next;

		ptemp->s_next = NULL;
		ptemp->s_key = NULL;
		ptemp->s_procQ = NULL;
		return ptemp;	
	}
}

/*Rimuove dalla ASL il semaforo puntato da sem*/
semd_t *deAllocSem(semd_t **semd_h, semd_t *sem){
	if(*semd_h == sem){
		semd_t *removed = sem;
		*semd_h = sem->s_next;
		insertSEMList(&semdFree_h, sem); /*Reinserisce il semaforo nella semdFree*/
		return removed;
	}
	else 
		return deAllocSem(&((*semd_h)->s_next), sem);
}

/*Ritorna il primo PCB dalla coda dei processi bloccati(s_ProcQ) associata al SEMD della ASL con chiave key. 
Se tale descrittore non esiste nella ASL, restituisce NULL. 
Altrimenti,restituisce l’elemento rimosso. 
Se la coda dei processi bloccati per il semaforo diventa vuota, rimuove il descrittore del semaforo corrispondente dalla ASL e lo inserisce nella coda dei semafori liberi (semdFree).*/
pcb_t* removeBlocked(int *key){
	semd_t *semd_target = getSemd(key);
	
	if(semd_target){
		/*Il semaforo è nella ASL*/
		pcb_t *removed = removeProcQ(&(semd_target->s_procQ)); /*Rimuovo il primo PCB della sua coda di processi*/
		if (semd_target->s_procQ == NULL)
			deAllocSem(&semd_h, semd_target);
		return removed;	
	}
	else
		return NULL;
}


/*Restituisce il puntatore al pcb del primo processo bloccato sul semaforo, senza deaccordarlo. 
Se il semaforo non esiste restituisce NULL.*/
pcb_t* headBlocked(int *key){
	semd_t *semd_target = getSemd(key);

	if (semd_target)
		return headProcQ(semd_target->s_procQ);
	else
		return NULL;
}
/*Rimuove il pcb puntato da p dalla coda dei processi sul semaforo a lui associato (p->p_semkey) sulla ASL.
Se il processo non risulta bloccato (cioè è un errore), restituisce NULL, altrimenti restituisce il puntatore al processo*/
pcb_t* outBlocked(pcb_t *p){
	semd_t *semd_target = getSemd(p->p_semkey);

	pcb_t *blocked = outProcQ(&(semd_target->s_procQ), p);
	if(!blocked) /*Se p non esiste nella coda del semaforo*/
		return NULL;
	else 
		return blocked;
}


/*Rimuove il PCB puntato da p dalla coda del semaforo su cui e’ bloccato
outChildBlocked() termina anche tutti i processi discendenti di p.
L'eliminazione avviene visitando l'albero come una DFS*/
void outChildBlocked(pcb_t *p){
	semd_t *semd_target = getSemd(p->p_semkey);

	if(p->p_first_child != NULL)
		outChildBlocked(p->p_first_child);

	if (p->p_first_child == NULL && p->p_sib != NULL)
		outChildBlocked(p->p_sib);	

	if(p->p_first_child == NULL && p->p_sib	== NULL){
		/*Caso foglia*/
		outProcQ(&(semd_target->s_procQ), p); /*Rimuovo p dalla coda dei processi del suo semaforo*/
		outChild(p); /*Rimuovo p dalla lista dei figli del padre*/
		return;
	}
}

/*Richiama la funzione fun per ogni elemento della coda di processi del semaforo della ASL con chiave key*/
void forallBlocked(int *key, void fun(struct pcb_t *pcb, void *), void *arg){
	semd_t *semd_target = getSemd(key);

	if (!semd_target->s_procQ)
		return;
	else
		forallProcQ(semd_target->s_procQ, fun, arg);
}

