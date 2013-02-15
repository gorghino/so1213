semd_t semd_table[MAXPROC];
semd_t *semdFree_h;


/* ASL handling functions */

/*Inizializza la lista dei semdFree in modo da contenere tutti gli elementi della semdTable. Questo metodo viene invocato una volta sola durante l’inizializzazione della strutuura dati.*/
void initASL(){
	initASL_rec(0);
}

void initASL_rec(int count){
	if (count>MAXPROC)
		return;

	insertSEMList(semFree_h, &semd_table[count]);
	
	initASL_rec(count++);
}

/*Viene inserito il PCB puntato da p nella coda dei processi bloccati associata al SEMD con chiave key. Se il semaforo corrispondente non e’ presente nella ASL, alloca un nuovo SEMD dalla lista di quelli liberi (semdFree) e lo inserisce nella ASL, settando I campi in maniera opportuna. Se non e’ possibile allocare un nuovo SEMD perche’ la lista di quelli liberi e’ vuota, restituisce TRUE. In tutti gli altri casi,restituisce FALSE*/
int insertBlocked(int *key, pcb_t* p){
	semd_t *semdFree_p = semdFree_h;
	insertBlocked_rec(semdFree_p, key, p);
}
int insertBlocked_rec (semd_t *semdFree_p, int *key, pcb_t* p){
	if(!semdFree_p)
		
	if(semdFree_p->s_key == key)
		insertProcQ(&(semdFree_p->s_procQ), p);
	else insertBlocked_rec (semdFree_p->s_next, key, p);

}

pcb_t* removeBlocked(int *key);
pcb_t* outBlocked(pcb_t *p);
pcb_t* headBlocked(int *key);
void outChildBlocked(pcb_t *p);


/*
this function can be inlined on implemented as a preprocessor macro.*/
void forallBlocked(int *key, void fun(struct pcb_t *pcb, void *), void *arg);