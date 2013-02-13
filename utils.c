void insertList(pcb_t *pcblist_p, pcb_t *pcb_elem){
	if(pcblist_p == NULL){
		pcb_elem = pcblist_p;
		return;
		}
	else if (pcblist_p->p_next == NULL)
			pcblist_p->p_next = pcb_elem;
	else insertList(pcblist_p->p_next, pcb_elem);
}

int emptyList(pcb_t *pcblist_p){
	if(pcblist_p == NULL)
		return 1;
	else
		return 0;
}