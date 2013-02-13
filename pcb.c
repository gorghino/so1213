#include "utils.c"


pcb_t pcb_table[MAXPROC];
pcb_t *pcbfree_h;

void initPcbs_rec(int count);

/* PCB free list handling functions */


/*Inizializza la pcbFree in modo da contenere tutti gli elementi della pcb_table.*/
void initPcbs(void){
	initPcbs_rec(0);
}
	
void initPcbs_rec(int count){
	if (count>MAXPROC)
		return;

	insertList(pcbfree_h, &pcb_table[count]);
	
	initPcbs_rec(count++);
}

/*Inserisce il PCB puntato da p nella lista dei PCB liberi (pcbFree)*/	
void freePcb(pcb_t *p){
	insertList(pcbfree_h, p);
	return;
}


/*Restituisce NULL se la pcbFree e’ vuota. Altrimenti rimuove un elemento dalla pcbFree, inizializza tutti i 
campi (NULL/0) e restituisce l’elemento rimosso.*/
pcb_t *allocPcb(void){

	if(emptyList(pcbfree_h))
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
	if (!head)
		head = p; /*la queue è vuota, p è il primo elemento*/
	else if(p->priority < (*head)->priority && head != NULL )
			insertProcQ(&((*head)->p_next), p);
	else if(p->priority > (*head)->priority && head != NULL){
		/*Inserisco il puntatore di pcb_t*/
		p->p_next = (*head);
		(*head) = p;
	}
	else{ 
		/*Caso uguale, lo inserisco dopo, evito Starvation*/
		p->p_next = (*head)->p_next;
		(*head)-> = p;
	}
}

/*Restituisce l’elemento di testa della coda dei processi da head, SENZA RIMUOVERLO. Ritorna NULL se la coda non ha elementi.*/
pcb_t* headProcQ(pcb_t* head){

	if (!head)
		return NULL;
	else
		return (*head);
}


/*Rimuove il primo elemento dalla coda dei processi puntata da head. Ritorna NULL se la coda e’ vuota. Altrimenti ritorna il puntatore all’elemento rimosso dalla lista.*/
pcb_t* removeProcQ( pcb_t** head){
	if (!head)
		return NULL;
	else{
		pcb_t * temp = (*head);
		(*head) = (*head)->p_next;
		return temp;
		}
}



/* Rimuove il PCB puntato da p dalla coda dei processi puntata da head. Se p non e’ presente nella coda, restituisce NULL. (NOTA: p puo’ trovarsi in una posizione arbitraria della coda).*/
pcb_t* outProcQ( pcb_t** head, pcb_t *p){
	if (!head)
		return NULL;
	else{
		if((*head) == p)
			removeProcQ(head);
		else
			outProcQ(&((*head)->p->next), p)
	}
}


/*
this can be inlined or implemented as preprocessor macro
*/
void forallProcQ(struct pcb_t *head, void fun(struct pcb_t *pcb, void *), void *arg){
return;
}


/* Tree view functions */
void insertChild(pcb_t *parent, pcb_t *p){
return;
}
pcb_t* removeChild(pcb_t *p){
return;
}
pcb_t * outChild(pcb_t* p){
return;
}

