#include "utils.h"
#include "pcb.e"
#include "libumps.h"
#include "p1test.h"


pcb_t pcb_table[MAXPROC];
pcb_t *pcbfree_h = NULL;


/* PCB free list handling functions */


/*Inizializza la pcbFree in modo da contenere tutti gli elementi della pcb_table.*/
void initPcbs(void){
	initPcbs_rec(0);
}
	
void initPcbs_rec(int count){
	if (count>=MAXPROC)
		return;

	insertPCBList(&pcbfree_h, &pcb_table[count]);
	
	count++;
	initPcbs_rec(count);
}

/*Inserisce il PCB puntato da p nella lista dei PCB liberi (pcbFree)*/	
void freePcb(pcb_t *p){
	insertPCBList(&pcbfree_h, p);
	return;
}


/*Restituisce NULL se la pcbFree e’ vuota. Altrimenti rimuove un elemento dalla pcbFree, inizializza tutti i 
campi (NULL/0) e restituisce l’elemento rimosso.*/
pcb_t *allocPcb(void){

	if(emptyPCBList(&pcbfree_h))
		return NULL;
	else{
		pcb_t *ptemp = pcbfree_h;
		pcbfree_h = pcbfree_h->p_next;

		ptemp->p_next = NULL;
		ptemp->p_parent = NULL;
		ptemp->p_first_child = NULL;
		ptemp->p_sib = NULL;

		ptemp->p_s.entry_hi = 0;
		ptemp->p_s.cause = 0;
		ptemp->p_s.status = 0;
		ptemp->p_s.pc_epc = 0;
		ptemp->p_s.gpr[29] = 0;
		ptemp->p_s.hi = 0;
		ptemp->p_s.lo = 0;
		
		ptemp->priority = 0;
		ptemp->p_semkey = NULL;

		return ptemp;	
	}
}	

/* PCB priority queue handling functions */

void insertProcQ(pcb_t **head, pcb_t* p){
	if ((*head) == NULL){
		(*head) = p; /*la queue è vuota, p è il primo elemento*/
		}
	else if(p->priority < (*head)->priority && head != NULL ){
			insertProcQ(&((*head)->p_next), p);
			}
	else if(p->priority > (*head)->priority && head != NULL){
		/*Inserisco il puntatore di pcb_t*/
		p->p_next = (*head);
		(*head) = p;
	}
	/*Casi uguali, scorro fino all'ultimo uguale e lo inserisco*/
	else{
		/*Caso uguale, lo inserisco dopo, evito Starvation*/
		pcb_t *last_equal = getLast(head, p->priority);
		
		p->p_next = last_equal->p_next;
		
		last_equal->p_next = p;
	}
}

pcb_t *getLast(pcb_t **head, int priority){
	if( (*head)->p_next == NULL  || ((*head)->p_next)->priority != priority){

		return (*head);
		}
	else{
		return getLast(&((*head)->p_next), priority);
		}
}

/*Restituisce l’elemento di testa della coda dei processi da head, SENZA RIMUOVERLO. Ritorna NULL se la coda non ha elementi.*/
pcb_t* headProcQ(pcb_t* head){

	if (!head)
		return NULL;
	else
		return head;
}


/*Rimuove il primo elemento dalla coda dei processi puntata da head. Ritorna NULL se la coda e’ vuota. Altrimenti ritorna il puntatore all’elemento rimosso dalla lista.*/
pcb_t* removeProcQ(pcb_t** head){
	if ((*head) == NULL)
		return NULL;
	else{
		pcb_t *temp = (*head);
		(*head) = (*head)->p_next;
		return temp;
		}
}



/* Rimuove il PCB puntato da p dalla coda dei processi puntata da head. Se p non e’ presente nella coda, restituisce NULL. (NOTA: p puo’ trovarsi in una posizione arbitraria della coda).*/
pcb_t* outProcQ(pcb_t** head, pcb_t *p){
	if (head == NULL){
		return NULL;
		}
	else if((*head) == p){
		return removeProcQ(head);
	}
	else{
		return outProcQ(&((*head)->p_next), p);
	}
}


/*This can be inlined or implemented as preprocessor macro*/

/*Richiama la funzione fun per ogni elemento della lista puntata da head*/
void forallProcQ(struct pcb_t *head, void fun(struct pcb_t *pcb, void *), void *arg){
	if (head == NULL){
		return;
		}
	else{
		fun(head, arg);
		forallProcQ(head->p_next, fun, arg);
	}
}


/* Tree view functions */

/*Inserisce il PCB puntato da p come figlio del PCB puntato da parent*/
void insertChild(pcb_t *parent, pcb_t *p){
	if(parent->p_first_child == NULL){
		parent->p_first_child = p;
		p->p_parent = parent;
		}
	else
		insertSibling(parent->p_first_child, p);
}

/*Rimuove il primo figlio del PCB puntato da p. Se p non ha figli, restituisce NULL.*/
pcb_t* removeChild(pcb_t *p){
	if(p->p_first_child == NULL)
		return NULL;
	else{
		pcb_t *temp = p->p_first_child;
		p->p_first_child = temp->p_sib; /*Aggiorno firstchild di p con il nuovo sibling*/
		temp->p_sib = NULL;
		temp->p_parent = NULL;
		return temp;	
	}
}

/*Rimuove il PCB puntato da p dalla lista dei figli del padre. Se il PCB puntato da p non ha un padre, restuisce NULL. Altrimenti restituisce l’elemento rimosso (cioe’ p). A differenza della removeChild, p puo’ trovarsi in una posizione arbitraria (ossia non e’ necessariamente il primo figlio del padre).*/

pcb_t* outChild(pcb_t* p){
	pcb_t *list_child = (p->p_parent)->p_first_child;

	if(p->p_parent == NULL)
		return NULL;
	else if(list_child == p)
		return removeChild(p->p_parent);
	else{
		pcb_t *removed = outChild_rec(list_child, p);
		return removed;
		}
}

pcb_t* outChild_rec(pcb_t *list_child, pcb_t *p){
	if(list_child->p_sib == NULL)
		return NULL; /*p non esiste*/
	else if(list_child->p_sib == p){
		pcb_t *removed = list_child->p_sib;
		list_child->p_sib = removed->p_sib;
		removed->p_sib = NULL;
		removed->p_parent = NULL;
		return removed;
	}
		
	else
		return outChild_rec(list_child->p_sib, p);
}
