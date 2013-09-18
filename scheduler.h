#ifndef SYS_H_
#define SYS_H_

#include "const13_customized.h"

state_t scheduler[MAX_CPUS];
int stateCPU[MAX_CPUS];
void schedule();
void init();
void pota();
void increment_priority();

unsigned int pcb_Lock;

#endif
