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
void terminatePcb(pcb_t *p);
void outChildBlocked(pcb_t *p);

semd_t *allocSem();
semd_t *deAllocSem(semd_t **semd_h, semd_t *sem);

void forallBlocked(int *key, void fun(struct pcb_t *pcb, void *), void *arg);

#endif
