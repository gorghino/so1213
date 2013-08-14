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

#include "utils.h"
#include "libumps.h"
#include "const13.h"
#include "uMPStypes.h"

int numTimes = 0;

void main(){
	state_t* new_area = (state_t*) INT_NEWAREA;	
  	
	/*interrupt disabilitati, kernel mode e memoria virtuale spenta*/	
	
	new_area-­>pc_epc = new_area-­>reg_t9 = (memaddr)timerHandler;		
	new_area-­>status &= ~(STATUS_IEc|STATUS_KUc|STATUS_VMc);		
	new_area-­>reg_sp = RAMTOP;	
  	
	int status=0;	
	
	status |= (STATUS_IEc|STATUS_INT_UNMASKED);		
	setSTATUS(status);		
	SET_IT(5000000);	
	
	while(1){}	 
}

void *timerHandler(){
	
	int cause=getCAUSE();		
	if(CAUSE_IP_GET(cause, INT_TIMER)) {		
		numTimes++;
		if(numTimes	> 5)		
			HALT();	
		SET_IT(5000000);	
	
	}	
  
}	
  
