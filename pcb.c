#include <const13.h>
#include <types13.h>

pcb_t pcb_table[MAXPROC];
pcb_t pcb_free[];
pcb_t *pcbfree_h;

/* PCB free list handling functions */
void initPcbs(void){

	pcb_t *pcb_table_p = pcb_table; /*Sono necessari i puntatori a pcb_table/free anche se sono globali?*/
	pcb_t *pcb_free_p = pcb_free;

	initPcbs_rec(pcb_table_p, pcb_free_p, 0);
}
	
void initPcbs_rec(pcb_t *pcb_table_p, pcb_t *pcb_free_p, int count){
	if (count>MAXPROC)
		return;

	pcb_free[count]=pcb_table[count];

	if (count==0)
		pcbfree_h = pcb_free[0];

	initPcbs_rec(pcb_table_p, pcb_free_p, count++);
}
		
void freePcb(pcb_t *p){
	pcb_t *pcbfree_p = pcb_free_h;
	freePcb_rec(p, pcbfree_p); 
}

void freePcb_rec(pcb_t *p, pcb_t *pcbfree_p){

	if(pcbfree_p->p_next == NULL){
		pcbfree_p->p_next = p;
		return;
	}
	else {
		pcbfree_p = pcbfree_p->p_next;
		freePcb_rec(p, pcbfree_p);
	}
} 

pcb_t *allocPcb(void){

/*Restituisce NULL se la pcbFree e’ vuota. Altrimenti rimuove un elemento dalla pcbFree, inizializza tutti i 
campi (NULL/0) e restituisce l’elemento rimosso.*/

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

