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
#include "uMPStypes.h"
#include "const13.h"
#include "const13_customized.h"
#include "pcb.e"
#include "interrupts.h"
#include "scheduler.h"
#include "main.h"
#include "utils.h"

void interruptHandler(){
	pcb_t *unblocked; /* Puntatore a processo sbloccato */

	/* Otteniamo la causa dell'invocazione dell'interrupt
	   e l'ID del processore sul quale girava il processore mentre veniva
	   invocato l'interrupt */
	int cause = getCAUSE();
	int cpuID = getPRID();
  
	/* Inter processor interrupts */
	/*Se viene sollevato l'IPI interval, un IPI è stato inviato tra processi. 
		L'interrupt si limita a fare l'ack scrivendo nell'IPI's Inbox un valore trascurabile*/
	if(CAUSE_IP_GET(cause, INT_IPI)) {
		ACK_IPI;
	}

	/* Processor Local Timer */
	/* Il Local Timer viene utilizzato nel caso scada il timeslice di un processo*/
	else if(CAUSE_IP_GET(cause, INT_LOCAL_TIMER)) {
		setTIMER(SCHED_TIME_SLICE);
	}

	/* Bus (Interval Timer) */
	/* Il Bus Interval timer è utilizzato per sbloccare ogni SCHED_PSEUDO_CLOCK ms i processi bloccati sulla WAIT_CLOCK, sul
	semaforo dello PseudoClock*/
	else if(CAUSE_IP_GET(cause, INT_TIMER)) {
		/*Exctract pcb and put them into the ready queue*/
		lock(MUTEX_CLOCK);
		while ((unblocked = V(&pseudo_clock))){
			insertProcQ(&ready_queue[unblocked->numCPU], unblocked);
		}
		SET_IT(SCHED_PSEUDO_CLOCK);
		unlock(MUTEX_CLOCK);
	}

	/* Disk Devices */
	else if(CAUSE_IP_GET(cause, INT_DISK)) {
		/*Scandiamo la INT_BITMAP_DISKDEVICE per identificare quale disk ha sollevato l'interrupt*/
		int devicenumber = findDeviceNumber((memaddr*)INT_BITMAP_DISKDEVICE);
		if( (unblocked = V(&sem_disk[devicenumber])) != NULL){
				/*Se un processo ha chiamato la SYS dei DISK, V() lo sblocca e aggiorna lo status*/
				unblocked->p_s.reg_v0 = *DEVICE_STATUS(INT_DISK, devicenumber);
				insertProcQ(&ready_queue[unblocked->numCPU], unblocked);
			}
		/* ACK al device */
		*DEVICE_COMMAND(INT_DISK, devicenumber) = DEV_C_ACK;
	}

	/* Tape Devices */
	else if(CAUSE_IP_GET(cause, INT_TAPE)) {
		/*Scandiamo la INT_BITMAP_TAPEDEVICE per identificare quale tape ha sollevato l'interrupt*/
		int devicenumber = findDeviceNumber((memaddr*)INT_BITMAP_TAPEDEVICE);
		if( (unblocked = V(&sem_tape[devicenumber])) != NULL){
				/*Se un processo ha chiamato la SYS dei TAPE, V() lo sblocca e aggiorna lo status*/
				unblocked->p_s.reg_v0 = *DEVICE_STATUS(INT_TAPE, devicenumber);
				insertProcQ(&ready_queue[unblocked->numCPU], unblocked);
			}
		/* ACK al device */
		*DEVICE_COMMAND(INT_TAPE, devicenumber) = DEV_C_ACK;
	}

	/* Network (Ethernet) Devices */
	else if(CAUSE_IP_GET(cause, INT_UNUSED)) {
		/*Scandiamo la INT_BITMAP_NETDEVICE per identificare quale network device ha sollevato l'interrupt*/
		int devicenumber = findDeviceNumber((memaddr*)INT_BITMAP_NETDEVICE);
		if( (unblocked = V(&sem_ethernet[devicenumber])) != NULL){
				/*Se un processo ha chiamato la SYS dei TAPE, V() lo sblocca e aggiorna lo status*/
				unblocked->p_s.reg_v0 = *DEVICE_STATUS(INT_UNUSED, devicenumber);
				insertProcQ(&ready_queue[unblocked->numCPU], unblocked);
			}
		/* ACK al device */
		*DEVICE_COMMAND(INT_UNUSED, devicenumber) = DEV_C_ACK;
	}

	/* Printer Devices */
	else if(CAUSE_IP_GET(cause, INT_PRINTER)) {
		/*Scandiamo la INT_BITMAP_PRINTERDEVICE per identificare quale printer ha sollevato l'interrupt*/
		int devicenumber = findDeviceNumber((memaddr*)INT_BITMAP_PRINTERDEVICE);
		if( (unblocked = V(&sem_ethernet[devicenumber])) != NULL){
				/*Se un processo ha chiamato la SYS dei TAPE, V() lo sblocca e aggiorna lo status*/
				unblocked->p_s.reg_v0 = *DEVICE_STATUS(INT_PRINTER, devicenumber);
				insertProcQ(&ready_queue[unblocked->numCPU], unblocked);
			}
		/* ACK al device */
		*DEVICE_COMMAND(INT_PRINTER, devicenumber) = DEV_C_ACK;
	}

	/*Gestione dei terminali*/
	else if(CAUSE_IP_GET(cause, INT_TERMINAL)) {
		/*Scandiamo la BITMAP_TERMINALDEVICE per identificare quale terminale ha sollevato l'interrupt*/
		int devicenumber = findDeviceNumber((memaddr*)INT_BITMAP_TERMINALDEVICE);

		/*Trovato il terminale, verifichiamo se è stato sollevato per leggere o scrivere*/
		if((*TERMINAL_RECV_STATUS(INT_TERMINAL, devicenumber) & STATUSMASK) != DEV_S_READY) {
			/*Caso read*/
			if( (unblocked = V(&sem_terminal_read[devicenumber])) != NULL){
				/*Se un processo ha chiamato la SYS8, V() lo sblocca e aggiorna lo status*/	
				unblocked->p_s.reg_v0 = *TERMINAL_RECV_STATUS(INT_TERMINAL, devicenumber);
				insertProcQ(&ready_queue[unblocked->numCPU], unblocked);
			}
			else
				/*Avviene prima l'INTERRUPT della syscall. "Store off" dello status nella struttura globale*/	
				device_read_response[devicenumber] = *TERMINAL_RECV_STATUS(INT_TERMINAL, devicenumber);

			/*Invio l'acknowledge al terminale*/
			*TERMINAL_RECV_COMMAND(INT_TERMINAL, devicenumber) = DEV_C_ACK;
		}
		if((*TERMINAL_TRANSM_STATUS(INT_TERMINAL, devicenumber) & STATUSMASK) != DEV_S_READY) {
			/*Caso write*/
			if( (unblocked = V(&sem_terminal_write[devicenumber])) != NULL){
				/*Se un processo ha chiamato la SYS8, V() lo sblocca e aggiorna lo status*/	
				unblocked->p_s.reg_v0 = *TERMINAL_TRANSM_STATUS(INT_TERMINAL, devicenumber);
				insertProcQ(&ready_queue[unblocked->numCPU], unblocked);
			}
			else
				/*Avviene prima l'INTERRUPT della syscall. "Store off" lo status nella struttura globale*/
				device_write_response[devicenumber] = *TERMINAL_TRANSM_STATUS(INT_TERMINAL, devicenumber);

			/*Invio l'acknowledge al terminale*/
			*TERMINAL_TRANSM_COMMAND(INT_TERMINAL, devicenumber) = DEV_C_ACK;
		}
	}

	/*Se l'INTERRUPT è stato sollevato mentre un processo era in stato RUNNING, lo reinserisco nella sua ReadyQueue*/
	if(current_process[cpuID] != NULL){
			cpuID > 0 ? copyState(&new_old_areas[cpuID][INT_OLDAREA_INDEX], &(current_process[cpuID]->p_s)) : \
						copyState(((state_t*)INT_OLDAREA), &(current_process[cpuID]->p_s));

			insertProcQ(&ready_queue[current_process[cpuID]->numCPU], current_process[cpuID]);	
	}

	/*Il controllo torna allo scheduler*/
	LDST(&scheduler[cpuID]);
}
