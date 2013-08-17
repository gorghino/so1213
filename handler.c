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
#include "libumps.h"
#include "const13.h"
#include "uMPStypes.h"

extern void addokbuf(char *strp);

state_t *sysBp_old = (state_t *)SYSBK_OLDAREA;
state_t *pgmTrap_old = (state_t *)PGMTRAP_OLDAREA;
state_t *tlbTrap_old = (state_t *)TLB_OLDAREA;

void tlbHandler(){
	addokbuf("tlbHandler: Panico!");
	PANIC();
}
void trapHandler(){
	addokbuf("trapHandler: Panico!");
	PANIC();
}

void syscallHandler(){
	int cause = CAUSE_EXCCODE_GET(getCAUSE());
	switch(cause){
		case EXC_SYSCALL: 
			addokbuf("SYSCALL\n"); 
			switch(sysBp_old->reg_a0){
				case CREATEPROCESS: addokbuf("CREATEPROCESS\n"); break;
				case TERMINATEPROCESS: addokbuf("TERMINATEPROCESS\n"); break;
				case VERHOGEN: addokbuf("VERHOGEN\n"); break;
				case PASSEREN: addokbuf("PASSEREN\n"); break;
				case SPECTRAPVEC: addokbuf("SPECTRAPVEC\n"); break;
				case GETCPUTIME: addokbuf("GETCPUTIME\n"); break;
				case WAITCLOCK: addokbuf("WAITCLOCK\n"); break;
				case WAITIO: addokbuf("WAITIO\n"); break;
			}
		break;
		case EXC_BREAKPOINT: addokbuf("BREAKPOINT\n"); break;
	}
	sysBp_old->pc_epc += 4; 
	LDST(sysBp_old); 
}
