#ifndef SYS_H_
#define SYS_H_

#include "const13.h"
#include "const13_customized.h"
#include "uMPStypes.h"

state_t scheduler[MAX_CPUS];

void schedule();
void init();
extern void increment_priority();

#endif
