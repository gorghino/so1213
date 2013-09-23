/*  Copyright (C) 2013 Aguiari Davide, Giacò Jacopo, Trotta Marco
 *  Authors: Aguiari Davide davide.aguiari@studio.unibo.it
 *        	 Giacò Jacopo jacopo.giaco@studio.unibo.it
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

#include "handler.h"
#include "pcb.e"
#include "asl.e"
#include "utils.h"
#include "libumps.h"
#include "const13.h"
#include "uMPStypes.h"
#include "const13_customized.h"
#include "main.h"

/*Entry point del sistema. Main() inizializza e popola le NewOldArea della ROM e le NewOldArea delle CPU > 0
	Inizializza le strutture dati di fase1, i semafori dei device e chiama lo scheduler*/
int main(){
	int i = 0;

	/*Init NewOldArea della ROM*/
	initNewOldArea((state_t *)INT_NEWAREA, (memaddr) interruptHandler, 0);
	initNewOldArea((state_t *)TLB_NEWAREA, (memaddr)tlbHandler, 0);
	initNewOldArea((state_t *)PGMTRAP_NEWAREA, (memaddr) trapHandler, 0);
	initNewOldArea((state_t *)SYSBK_NEWAREA, (memaddr) syscallHandler, 0);
	
	/*Init NewOldArea delle CPU > 0.*/
    for (i=1; i<GET_NCPU;i++){
    	initNewOldArea(&new_old_areas[i][INT_NEWAREA_INDEX], (memaddr) interruptHandler, i);
    	initNewOldArea(&new_old_areas[i][TLB_NEWAREA_INDEX], (memaddr) tlbHandler, i);
    	initNewOldArea(&new_old_areas[i][PGMTRAP_NEWAREA_INDEX], (memaddr) trapHandler, i);
    	initNewOldArea(&new_old_areas[i][SYSBK_NEWAREA_INDEX], (memaddr) syscallHandler, i);
    }

    /*Inizializzo le strutture dati di fase1*/
    initPcbs();
    initASL();

    /*Inizializzo tutte le variabili del nucleo: Process Count, Soft-block Count, Ready Queues, and Current Process.*/
	for (i=0; i<GET_NCPU; i++){
		process_count[i] = 0; /*Counter processi attivi*/  	
    	ready_queue[i] = NULL; /*Coda dei processi in stato ready*/
    	current_process[i] = NULL; /*Puntatore al processo in esecuzione*/
    }
    softBlock_count = 0; /*Contatore processi in stato wait*/

    /*Inizializzo i semafori mantenuti dal nucleo
    	Vengono impostati a 0 i semafori dei device (uno per device), due per ogni terminal (scrittura/lettura) e
    	il semaforo dello pseudoClock.*/
	for(i=0;i<DEV_PER_INT;i++){
		sem_disk[i] = 0;
		sem_tape[i] = 0;
		sem_ethernet[i] = 0;
		sem_printer[i] = 0;
		sem_terminal_read[i] = 0;
		sem_terminal_write[i] = 0;
	}
	pseudo_clock = 0; 

	/*Inizializzo i semafori delle CAS:
		0-MAX_CPUS: Scheduler
		MAX_CPUS+1: PV
		MAX_CPUS+2: PseudoClock*/

	for (i=0;i<NUM_SEM_CUSTOM; i++)
		semArray[i] = 1;

	/*Inizializzo lo scheduler*/
	init();

	/*Error: Non uscirò mai dallo scheduler*/
	PANIC();
	return -1;
}
