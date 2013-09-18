#ifndef M_H
#define M_H

#include "libumps.h"
#include "const13.h"
#include "const13_customized.h"
#include "base.h"

extern void addokbuf(char *strp);
extern void test();
extern void schedule();
extern void init();
extern void interruptHandler();
extern void tlbHandler();
extern void trapHandler();
extern void syscallHandler();

int sem_disk[8];
int sem_tape[8];
int sem_ethernet[8];
int sem_printer[8];
int sem_terminal_read[8];
int sem_terminal_write[8];

int pseudo_clock;
state_t new_old_areas[MAX_CPUS][8];

int semArray[3];

#endif
