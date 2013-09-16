#ifndef SYS_H_
#define SYS_H_

#define	MAX_CPUS 1

state_t scheduler[MAX_CPUS];
void schedule();
void init();
void pota();
void increment_priority();

unsigned int pcb_Lock;

#endif
