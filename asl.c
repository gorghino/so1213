#include "asl.e"
#include "utils.h"

semd_t semd_table[MAXPROC];
semd_t *semdFree_h;
semd_t *semd_h;

#define TRUE 1
#define FALSE 0




/* ASL handling functions */

/*Inizializza la lista dei semdFree in modo da contenere tutti gli elementi della semdTable. Questo metodo viene invocato una volta sola durante l’inizializzazione della strutuura dati.*/
void initASL(){
	initASL_rec(0);
}

void initASL_rec(int count){
	if (count>MAXPROC)
		return;

	insertSEMList(&semdFree_h, &semd_table[count]);
	
	count++;
	initASL_rec(count);
}

semd_t* getSemd(int *key){
	return getSemd_rec(&semd_h, key);
}

semd_t* getSemd_rec(semd_t **semd_h, int* key){
	if(*semd_h == NULL){
		return NULL;
	}
	if((*semd_h)->s_key == key){
		return *semd_h;
		}
	else 
		return getSemd_rec(&((*semd_h)->s_next), key);
}

/*Viene inserito il PCB puntato da p nella coda dei processi bloccati associata al SEMD con chiave key. Se il semaforo corrispondente non e’ presente nella ASL, alloca un nuovo SEMD dalla lista di quelli liberi (semdFree) e lo inserisce nella ASL, settando I campi in maniera opportuna. Se non e’ possibile allocare un nuovo SEMD perche’ la lista di quelli liberi e’ vuota, restituisce TRUE. In tutti gli altri casi,restituisce FALSE*/
int insertBlocked(int *key, pcb_t* p){
		semd_t *semd_target = getSemd(key);
		if(semd_target == NULL){
			semd_target = allocSem(); /*semd_target ora punta al SEMD allocato nella ASL*/
			if (semd_target == NULL) /*Se la semdFree è vuota restituisco TRUE*/
				return TRUE;
			semd_target->s_key = key;
			
			insertSEMList(&semd_h, semd_target);
			p->p_semkey = key;
			
			insertProcQ(&(semd_target->s_procQ), p);
			return FALSE;
			}
		else{
			insertProcQ(&(semd_target->s_procQ), p);
			return FALSE;
		}	
	}

semd_t *allocSem(){
	if(emptySEMList(semdFree_h))
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

semd_t *deAllocSem(semd_t **semd_h, semd_t *sem){
	if(*semd_h == sem){
		semd_t *removed = sem;
		*semd_h = sem->s_next;
		insertSEMList(&semdFree_h, sem);
		return removed;
	}
	else 
		return deAllocSem(&((*semd_h)->s_next), sem);
}
/*Ritorna il primo PCB dalla coda dei processi bloccati(s_ProcQ) associata al SEMD della ASL con chiave key. Se tale descrittore non esiste nella ASL, restituisce NULL. Altrimenti,restituisce l’elemento rimosso. Se la coda dei processi bloccati per il semaforo diventa vuota, rimuove il descrittore corrispondente dalla ASL e lo inserisce nella coda dei descrittori liberi (semdFree).*/
pcb_t* removeBlocked(int *key){
	semd_t *semd_target = getSemd(key);
	if(semd_target){
		pcb_t *removed = removeProcQ(&(semd_target->s_procQ));
		if (semd_target->s_procQ == NULL)
			deAllocSem(&semd_h, semd_target);
		return removed;	
	}
	else
		return NULL;
}


/*restituisce il puntatore al pcb del primo processo bloccato sul semaforo, senza deaccordarlo. Se il semaforo non esiste restituisce NULL.*/
pcb_t* headBlocked(int *key){
	semd_t *semd_target = getSemd(key);
	if (semd_target)
		return headProcQ(semd_target->s_procQ);
	else
		return NULL;
}

/* Remove the ProcBlk pointed to by p from the process queue associated with p’s semaphore (p→p_semkey) on the ASL. 
If ProcBlk pointed to by p does not appear in the process queue associated with p’s semaphore, which is an error condition, return NULL; otherwise,return p. */
pcb_t* outBlocked(pcb_t *p){
	semd_t *semd_target = getSemd(p->p_semkey);
	pcb_t *blocked = outProcQ(&(semd_target->s_procQ), p);
	if(!blocked) /*Se p non esiste nella coda del semaforo*/
		return NULL;
	else 
		return blocked;
}


/*Rimuove il PCB puntato da p dalla coda del semaforo su cui e’ bloccato*/
/* Si specifica che la terminazione outChildBlocked deve eliminare anche tutti i processi discendenti.*/
void outChildBlocked(pcb_t *p){
	semd_t *semd_target = getSemd(p->p_semkey);
	outProcQ(&(semd_target->s_procQ), p);
	if(p->p_first_child == NULL)
		return;
	else
		outChildBlocked(p->p_first_child);
}


/*
this function can be inlined on implemented as a preprocessor macro.*/
void forallBlocked(int *key, void fun(struct pcb_t *pcb, void *), void *arg){
	semd_t *semd_target = getSemd(key);
	if (!semd_target->s_procQ)
		return;
	else
		forallProcQ(semd_target->s_procQ, fun, arg);	
}
