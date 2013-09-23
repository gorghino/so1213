#ifndef M_H
#define M_H

#include "libumps.h"
#include "const13.h"
#include "const13_customized.h"
#include "base.h"

extern void test();
extern void schedule();
extern void init();
extern void interruptHandler();
extern void tlbHandler();
extern void trapHandler();
extern void syscallHandler();
extern void initNewOldArea();

state_t new_old_areas[MAX_CPUS][8];
pcb_t *ready_queue[MAX_CPUS];
pcb_t *current_process[MAX_CPUS];
int process_count[MAX_CPUS];
int softBlock_count;
int stateCPU[MAX_CPUS]; 	

/*Semafori*/
int sem_disk[8];
int sem_tape[8];
int sem_ethernet[8];
int sem_printer[8];
int sem_terminal_read[8];
int sem_terminal_write[8];
int pseudo_clock;

U32 semArray[MAX_CPUS+2];
U32 SOFT_LOCK;




#endif
