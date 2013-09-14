#include "libumps.h"
#include "const13.h"

#define WAITING 0
#define RUNNING 1
#define	MAX_CPUS 1

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

int pseudo_clock[MAX_CPUS];

int stateCPU[MAX_CPUS]; 
