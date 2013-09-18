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
#include "uMPStypes.h"
#include "pcb.e"
#include "const13_customized.h"
#include "scheduler.h"
#include "main.h"

extern void addokbuf(char *strp);
extern pcb_t *current_process[MAX_CPUS];
extern pcb_t *ready_queue[MAX_CPUS];
extern int process_count[MAX_CPUS];


void interruptHandler(){
	char buffer[1024];
	int cause=getCAUSE();
	termreg_t *DEVREG;
	pcb_t * unblocked;

	int cpuID = getPRID();
  
	/* Inter processor interrupts */
	if(CAUSE_IP_GET(cause, 0)) {
		HALT();
	  itoa(0, buffer, 10);
	}
	
	/* Processor Local Timer */
	/*The processor Local Timer is useful for generating interrupts*/
	else if(CAUSE_IP_GET(cause, 1)) {
	  //itoa(1, buffer, 10);*
		setTIMER(4000);
		//insertProcQ(&ready_queue[cpuID], current_process[cpuID]);	
		//current_process[cpuID] = NULL;
	}
	
	/* Bus (Interval Timer) */
	/*For Interval Timer interrupts that represent a pseudo-clock tick (see Section 3.7.1), perform the V 
	operation on the nucleus maintained pseudo-clock timer semaphore.*/
	else if(CAUSE_IP_GET(cause, INT_TIMER)) {
		/*Exctract pcb and put them into the ready queue*/
		lock(semClock);
		while ((unblocked = V(&pseudo_clock))){

			insertProcQ(&ready_queue[unblocked->numCPU], unblocked);
		}
		SET_IT(SCHED_PSEUDO_CLOCK);
		unlock(semClock);

	}
	
	/* Disk Devices */
	else if(CAUSE_IP_GET(cause, INT_DISK)) {
		int* diskdevice=(int*)INT_BITMAP_DISKDEVICE;
		/*itoa(*diskdevice, buffer, 10);
		addokbuf(buffer);*/
	}
	
	/* Tape Devices */
	else if(CAUSE_IP_GET(cause, INT_TAPE)) {
		int* tapdevice=(int*)INT_BITMAP_TAPEDEVICE;
		/*itoa(*tapdevice, buffer, 10);
		addokbuf(buffer);*/
	}
	
	/* Network (Ethernet) Devices */
	else if(CAUSE_IP_GET(cause, INT_UNUSED)) {
		int* netdevice=(int*)INT_BITMAP_NETDEVICE;
		/*itoa(*netdevice, buffer, 10);
		addokbuf(buffer);*/
	}
	
	/* Printer Devices */
	else if(CAUSE_IP_GET(cause, INT_PRINTER)) {
		int* printdevice=(int*)INT_BITMAP_PRINTERDEVICE;
		/*itoa(*printdevice, buffer, 10);
		addokbuf(buffer);*/
	}
	
	/* Terminal Devices */
	else if(CAUSE_IP_GET(cause, INT_TERMINAL)) {

		int* terminaldevice=(int*)INT_BITMAP_TERMINALDEVICE;
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
		
		int devicenumber = finddevicenumber((memaddr*)INT_BITMAP_TERMINALDEVICE);

		if((*TERMINAL_RECV_STATUS(INT_TERMINAL, devicenumber) & STATUSMASK) != DEV_S_READY) {

			if( (unblocked = V(&sem_terminal_write[devicenumber])) != NULL){	
				unblocked->p_s.reg_v0 = *TERMINAL_RECV_STATUS(INT_TERMINAL, devicenumber);
				insertProcQ(&ready_queue[unblocked->numCPU], unblocked);
			}
			else{
				device_read_response[devicenumber] = *TERMINAL_RECV_STATUS(INT_TERMINAL, devicenumber);
			}

			*TERMINAL_RECV_COMMAND(INT_TERMINAL, devicenumber) = DEV_C_ACK;
		}


		//QUIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
		if((*TERMINAL_TRANSM_STATUS(INT_TERMINAL, devicenumber) & STATUSMASK) != DEV_S_READY) {
			
			if( (unblocked = V(&sem_terminal_read[devicenumber])) != NULL){	
				unblocked->p_s.reg_v0 = *TERMINAL_TRANSM_STATUS(INT_TERMINAL, devicenumber);
				insertProcQ(&ready_queue[unblocked->numCPU], unblocked);
			}
			else
				device_write_response[devicenumber] = *TERMINAL_TRANSM_STATUS(INT_TERMINAL, devicenumber);

			*TERMINAL_TRANSM_COMMAND(INT_TERMINAL, devicenumber) = DEV_C_ACK;
		}
		
	}

	if(current_process[cpuID] != NULL){
			if(cpuID > 0)
				copyState(&new_old_areas[cpuID][0], &(current_process[cpuID]->p_s));
			else
				copyState(((state_t*)INT_OLDAREA), &(current_process[cpuID]->p_s));

			insertProcQ(&ready_queue[current_process[cpuID]->numCPU], current_process[cpuID]);	
			//LDST(&current_process[cpuID]->p_s); 
	}

	LDST(&scheduler[cpuID]);
}
