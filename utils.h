#include "const13.h"
#include "types13.h"

#define STATUSMASK 0xFF
#define TRUE 1
#define FALSE 0
#define	MAX_CPUS 1

pcb_t pcb_table[MAXPROC];
semd_t semd_table[MAXPROC];

semd_t *semdFree_h;
semd_t *semd_h;
pcb_t *pcbfree_h;

unsigned int device_read_response[DEV_PER_INT];
unsigned int device_write_response[DEV_PER_INT];

void insertPCBList(pcb_t **pcblist_p, pcb_t *pcb_elem);
void insertSEMList(semd_t **semlist_p, semd_t *sem_elem);
int emptyPCBList(pcb_t **pcblist_p);
int emptySEMList(semd_t **semlist_p);
void insertSibling(pcb_t *firstchild, pcb_t *p);

void strreverse();
void itoa();
