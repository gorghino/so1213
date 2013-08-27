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

/* TERMINAL */
#include "utils.h"
#include "libumps.h"
#include "const13.h"
#include "const13_customized.h"
#include "uMPStypes.h"

extern void addokbuf(char *strp);

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
	if(CAUSE_IP_GET(cause, INT_TIMER)) {
	  itoa(2, buffer, 10);
	}
	
	/* Disk Devices */
	if(CAUSE_IP_GET(cause, INT_DISK)) {
	  itoa(3, buffer, 10);
	}
	
	/* Tape Devices */
	if(CAUSE_IP_GET(cause, INT_TAPE)) {
	  itoa(4, buffer, 10);
	}
	
	/* Network (Ethernet) Devices */
	if(CAUSE_IP_GET(cause, INT_UNUSED)) {
	  itoa(5, buffer, 10);
	}
	
	/* Printer Devices */
	if(CAUSE_IP_GET(cause, INT_PRINTER)) {
	  itoa(6, buffer, 10);
	}
	
	/* Terminal Devices */
	if(CAUSE_IP_GET(cause, INT_TERMINAL)) {
		int* terminaldevice=(memaddr)INT_BITMAP_TERMINALDEVICE;
		itoa(*terminaldevice, buffer, 10);
		addokbuf(buffer);
	}
}
