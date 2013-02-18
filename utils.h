#include "const13.h"
#include "types13.h"

pcb_t pcb_table[MAXPROC];
pcb_t *pcbfree_h;

void insertPCBList(pcb_t **pcblist_p, pcb_t *pcb_elem);
void insertSEMList(semd_t **semlist_p, semd_t *sem_elem);
int emptyPCBList(pcb_t *pcblist_p);
int emptySEMList(semd_t *semlist_p);
void insertSibling(pcb_t *firstchild, pcb_t *p);

