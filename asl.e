#ifndef ASL_E
#define ASL_E

#include "const13.h"
#include "types13.h"
#include "pcb.e"

/* ASL handling functions */
semd_t* getSemd(int *key);
semd_t* getSemd_rec(semd_t **semd_h_temp, int* key);
void initASL();
void initASL_rec(int count);

int insertBlocked(int *key, pcb_t* p);
pcb_t* removeBlocked(int *key);
pcb_t* outBlocked(pcb_t *p);
pcb_t* headBlocked(int *key);
void outChildBlocked(pcb_t *p);

semd_t *allocSem();
semd_t *deAllocSem(semd_t **semd_h, semd_t *sem);



/*
this function can be inlined on implemented as a preprocessor macro.*/
void forallBlocked(int *key, void fun(struct pcb_t *pcb, void *), void *arg);


#endif
