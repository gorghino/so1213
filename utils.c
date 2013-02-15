void insertPCBList(pcb_t *pcblist_p, pcb_t *pcb_elem){
	if(pcblist_p == NULL){
		pcb_elem = pcblist_p;
		return;
		}
	else if (pcblist_p->p_next == NULL)
			pcblist_p->p_next = pcb_elem;
	else insertPCBList(pcblist_p->p_next, pcb_elem);
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

int emptyList(pcb_t *pcblist_p){
	if(pcblist_p == NULL)
		return 1;
	else
		return 0;
}

void insertSibling(pcb_t *firstchild, pcb_t *p){
	if (firstchild->p_sib == NULL){
		firstchild->p_sib = p;
		p->parent = firstchild->parent;
		}
	else insertSibling(firstchild->p_sib, p);
}