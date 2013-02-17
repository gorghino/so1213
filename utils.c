#include "utils.h"
#include "p1test.h"


void insertPCBList(pcb_t **pcblist_p, pcb_t *pcb_elem){
	if(*pcblist_p == NULL){
		addokbuf("Aggiungo il primo\n");
		*pcblist_p = pcb_elem;
		pcb_elem->p_next = NULL;
		return;
	}
	else if ((*pcblist_p)->p_next == NULL) {
		addokbuf("Aggiungo i restanti fino a MAXPROC-1\n");
		(*pcblist_p)->p_next = pcb_elem;
		pcb_elem->p_next = NULL;
	}
	else insertPCBList(&((*pcblist_p)->p_next), pcb_elem);
}

void insertSEMList(semd_t *semlist_p, semd_t *sem_elem){
	if(semlist_p == NULL){
		sem_elem = semlist_p;
		return;
	}
	else if (semlist_p->s_next == NULL)
		semlist_p->s_next = sem_elem;
	else insertSEMList(semlist_p->s_next, sem_elem);
}

int emptyPCBList(pcb_t *pcblist_p){
	if(pcblist_p == NULL)
		return 1;
	else
		return 0;
}

int emptySEMList(semd_t *semlist_p){
	if(semlist_p == NULL)
		return 1;
	else
		return 0;
}

void insertSibling(pcb_t *firstchild, pcb_t *p){
	if (firstchild->p_sib == NULL){
		firstchild->p_sib = p;
		p->p_parent = firstchild->p_parent;
		}
	else insertSibling(firstchild->p_sib, p);
}
