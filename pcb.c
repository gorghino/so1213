#include <const13.h>
#include <types13.h>

pcb_t pcb_table[MAXPROC];
pcb_t pcb_free[MAXPROC];
pcb_t *pcbfree_h;

/* PCB free list handling functions */


/*Inizializza la pcbFree in modo da contenere tutti gli elementi della pcb_table.*/
void initPcbs(void){
	initPcbs_rec(0);
}
	
void initPcbs_rec(int count){
	if (count>MAXPROC)
		return;

	pcb_free[count]=pcb_table[count];

	if (count==0)
		pcbfree_h = pcb_free[0];

	initPcbs_rec(count++);
}

/*Inserisce il PCB puntato da p nella lista dei PCB liberi (pcbFree)*/	
void freePcb(pcb_t *p){

	pcb_t *pcbfree_p = pcb_free_h; /*Inizialmente pcbfree_p punta al primo PCB in pcbFree*/
	freePcb_rec(p, pcbfree_p); 
}

void freePcb_rec(pcb_t *p, pcb_t *pcbfree_p){

	if(pcbfree_p->p_next == NULL){
		pcbfree_p->p_next = p; /*Faccio puntare p dal primo PCB senza next*/
		return;
	}
	else {
		pcbfree_p = pcbfree_p->p_next;
		freePcb_rec(p, pcbfree_p);
	}
} 


/*Restituisce NULL se la pcbFree e’ vuota. Altrimenti rimuove un elemento dalla pcbFree, inizializza tutti i 
campi (NULL/0) e restituisce l’elemento rimosso.*/
pcb_t *allocPcb(void){

	if(pcb_free_h == NULL)
		return NULL;
	else{
		pcb_t *ptemp = pcb_free_h;
		pcb_free_h = pcb_free_h->p_next;

		ptemp->p_next = NULL;
		ptemp->p_parent = NULL;
		ptemp->p_first_child = NULL;
		ptemp->p_sib = NULL;

		ptemp->p_s = NULL;   
		ptemp->priority = 0;
		ptemp->p_semkey = NULL;

		return ptemp;	
	}
}	

