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
#include "pcb.e"

extern void addokbuf(char *strp);
extern pcb_t *current_process[MAX_CPUS];
extern pcb_t *ready_queue[MAX_CPUS];

void interruptHandler(){
	char buffer[1024];
	int cause=getCAUSE();
	termreg_t *DEVREG;

	int processor_id = getPRID(); 
  
	/* Inter processor interrupts */
	if(CAUSE_IP_GET(cause, 0)) {
	  itoa(0, buffer, 10);
	}
	
	/* Processor Local Timer */
	else if(CAUSE_IP_GET(cause, 1)) {
	  itoa(1, buffer, 10);
	}
	
	/* Bus (Interval Timer) */
	else if(CAUSE_IP_GET(cause, INT_TIMER)) {
		itoa(INT_TIMER, buffer, 10);
	}
	
	/* Disk Devices */
	else if(CAUSE_IP_GET(cause, INT_DISK)) {
		int* diskdevice=(memaddr)INT_BITMAP_DISKDEVICE;
		itoa(*diskdevice, buffer, 10);
		addokbuf(buffer);
	}
	
	/* Tape Devices */
	else if(CAUSE_IP_GET(cause, INT_TAPE)) {
		int* tapdevice=(memaddr)INT_BITMAP_TAPEDEVICE;
		itoa(*tapdevice, buffer, 10);
		addokbuf(buffer);
	}
	
	/* Network (Ethernet) Devices */
	else if(CAUSE_IP_GET(cause, INT_UNUSED)) {
		int* netdevice=(memaddr)INT_BITMAP_NETDEVICE;
		itoa(*netdevice, buffer, 10);
		addokbuf(buffer);
	}
	
	/* Printer Devices */
	else if(CAUSE_IP_GET(cause, INT_PRINTER)) {
		int* printdevice=(memaddr)INT_BITMAP_PRINTERDEVICE;
		itoa(*printdevice, buffer, 10);
		addokbuf(buffer);
	}
	
	/* Terminal Devices */
	else if(CAUSE_IP_GET(cause, INT_TERMINAL)) {
		int* terminaldevice=(memaddr)INT_BITMAP_TERMINALDEVICE;
		//itoa(*terminaldevice, buffer, 10);
		//addokbuf("Terminal\n");
		//addokbuf(buffer);
		//addokbuf("\n");
		//itoa(CAUSE_EXCCODE_GET(cause), buffer, 10);
		//addokbuf("Line\n");
		//addokbuf(buffer);
		//addokbuf("\n");
		//itoa(CAUSE_CE_GET(cause), buffer, 10);
		//addokbuf("Coprocessor\n");
		//addokbuf(buffer);
		//addokbuf("\n");
		/*
		DEVREG = (termreg_t *)DEV_REG(INT_TERMINAL);
		itoa(DEVREG->recv_command, buffer, 10);
		addokbuf(buffer);
		DEVREG->recv_command = DEV_C_ACK;
		itoa(DEVREG->recv_command, buffer, 10);
		addokbuf(buffer);*/
		
		int devicenumber;
		finddevicenumber(INT_BITMAP_TERMINALDEVICE, &devicenumber);

		
		//addokbuf("Device register\n");
		//itoa(devicenumber, buffer, 10);
		//addokbuf(buffer);
		//addokbuf("\n");
		if((*TERMINAL_RECV_STATUS(INT_TERMINAL, devicenumber) & STATUSMASK) != DEV_S_READY) {
			*TERMINAL_RECV_COMMAND(INT_TERMINAL, devicenumber) = DEV_C_ACK;
			device_read_response[devicenumber] = TERMINAL_RECV_STATUS(INT_TERMINAL, devicenumber);
		}

		if((*TERMINAL_TRANSM_STATUS(INT_TERMINAL, devicenumber) & STATUSMASK) != DEV_S_READY){
			*TERMINAL_TRANSM_COMMAND(INT_TERMINAL, devicenumber) = DEV_C_ACK;
			device_write_response[devicenumber] = TERMINAL_TRANSM_STATUS(INT_TERMINAL, devicenumber);
		}

		if(current_process[processor_id])
			current_process[processor_id] = (state_t*)INT_OLDAREA;
			/*if (processor_id > 0)
				copyState((&HEADER_AREAS[prid][CPU_INT_OLDAREA_INDEX]),(&current->p_s));
			else
				copyState((),(&current->p_s));*/
			
		insertProcQ(&ready_queue[processor_id], current_process);
	}
	LDST(current_process[processor_id]);
}
