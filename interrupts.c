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

void interruptHandler(){
  char buffer[1024];
  
  int cause=getCAUSE();
  
	/* Inter processor interrupts */
	if(CAUSE_IP_GET(cause, 0)) {
	  itoa(0, buffer, 10);
	}
	
	/* Processor Local Timer */
	if(CAUSE_IP_GET(cause, 1)) {
	  itoa(1, buffer, 10);
	}
	
	/* Bus (Interval Timer) */
	if(CAUSE_IP_GET(cause, 2)) {
	  itoa(2, buffer, 10);
	}
	
	/* Disk Devices */
	if(CAUSE_IP_GET(cause, 3)) {
	  itoa(3, buffer, 10);
	}
	
	/* Tape Devices */
	if(CAUSE_IP_GET(cause, 4)) {
	  itoa(4, buffer, 10);
	}
	
	/* Network (Ethernet) Devices */
	if(CAUSE_IP_GET(cause, 5)) {
	  itoa(5, buffer, 10);
	}
	
	/* Printer Devices */
	if(CAUSE_IP_GET(cause, 6)) {
	  itoa(6, buffer, 10);
	}
	
	/* Terminal Devices */
	if(CAUSE_IP_GET(cause, 7)) {
	  itoa(7, buffer, 10);
	}
}
