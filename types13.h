#ifndef _TYPES13_H
#define _TYPES13_H

#include "uMPStypes.h"

/* Process Control Block (PCB) data structure*/
typedef struct pcb_t {
	/*process queue fields */
	struct pcb_t* p_next;

	/*process tree fields */
	struct pcb_t* p_parent;
	struct pcb_t* p_first_child;
	struct pcb_t* p_sib;

	/* process priority */
	int	priority;  
	int static_priority;

	/* processor state, etc */
	state_t	p_s;

	/* key of the semaphore on which the process is eventually blocked */
	int	*p_semkey;

	/*Tempo di inizio esecuzione*/
	int startTime;

	int numCPU;

	//stati array
	
	state_t*			states_array[6];

} pcb_t;

/* Semaphore Descriptor (SEMD) data structure*/
typedef struct semd_t {
	struct semd_t* s_next;

	/* Semaphore key*/
	int *s_key;

	/* Queue of PCBs blocked on the semaphore*/
	pcb_t *s_procQ;
} semd_t;

#endif
