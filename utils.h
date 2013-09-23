/*  Copyright (C) 2013 Aguiari Davide, Giacò Jacopo, Trotta Marco
 *  Authors: Aguiari Davide davide.aguiari@studio.unibo.it
 *			 Giacò Jacopo jacopo.giaco@studio.unibo.it
 					Trotta Marco marco.trotta2@studio.unibo.it
 							
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UTILS_H_
#define UTILS_H_

#include "const13.h"
#include "const13_customized.h"
#include "base.h"
#include "libumps.h"
#include "uMPStypes.h"
#include "types13.h"

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
void lock(int semkey);
void unlock(int semkey);
int P(int *key, pcb_t *process);
pcb_t* V(int *key);
int* findAddr(int lineNumber, int deviceNumber);
int findDeviceNumber(memaddr* bitmap);
void initNewOldArea(state_t * area, memaddr handler, int offset);
void increment_priority(struct pcb_t *pcb, void* pt);
void cpuIdle();
void killMe();
int isUserMode(int cpuID);
void plusSoftCounter();
void lessSoftCounter();
int checkSemaphore(pcb_t *p);

#endif
