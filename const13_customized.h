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

#ifndef CONST_H_CUSTOM
#define CONST_H_CUSTOM

/**************************************************************************** 
 *
 * This header file contains the global customized constant & macro definitions.
 * 
 ****************************************************************************/

#include "base.h"

/***** CPU *****/
#define	MAX_CPUS 16

/*CPU state*/
#define STOPPED 0
#define RUNNING 1

/*CPU areas*/
#define INT_OLDAREA_INDEX		0
#define INT_NEWAREA_INDEX 		1
#define TLB_OLDAREA_INDEX 		2
#define TLB_NEWAREA_INDEX 		3
#define PGMTRAP_OLDAREA_INDEX	4
#define PGMTRAP_NEWAREA_INDEX	5
#define SYSBK_OLDAREA_INDEX 	6
#define SYSBK_NEWAREA_INDEX 	7


/*Pcb cpu areas*/
#define TLB_OLD 	0
#define TLB_NEW		1
#define PGMTRAP_OLD 2
#define PGMTRAP_NEW	3
#define SYSBK_OLD	4
#define SYSBK_NEW 	5		


/***** SEMAFORI *****/
/*Indici dei semafori custom di semArray utilizzati per la mutua esclusione*/ 
#define NUM_SEM_CUSTOM 4
#define MUTEX_SCHEDULER 0
#define MUTEX_SOFTBLOCK 1
#define MUTEX_PV 2
#define MUTEX_CLOCK 3

/***** INTERRUPTS *****/

#define STATUSMASK 0xFF
#define NUM_DEVICES 6

/*Inter-Processor Interrupt*/
#define INT_IPI 0
/*Local Timers Interrupt*/
#define INT_LOCAL_TIMER 1

/* Interrupting devices bitmaps addresses  */
#define INT_BITMAP_DISKDEVICE 0x1000003c
#define INT_BITMAP_TAPEDEVICE 0x10000040
#define INT_BITMAP_NETDEVICE 0x10000044
#define INT_BITMAP_PRINTERDEVICE 0x10000048
#define INT_BITMAP_TERMINALDEVICE 0x1000004C

/*Offset of registers */
#define DEV_STATUS 0x0
#define DEV_CMD 0x4
#define DEV_DATA0 0x8
#define DEV_DATA1 0xC

/*Offset of register terminal */
#define RECV_STATUS 0x0
#define RECV_COMMAND 0x4
#define TRANSM_STATUS 0x8
#define TRANSM_COMMAND 0xC

/*Devices registers addresses */
#define DEV_REG(int_n, dev_n) (DEV_REGS_START + ((int_n - 3) * 0x80) + (dev_n * 0x10))
#define DEVICE_STATUS(int_n, dev_n) (memaddr*)(DEV_REG(int_n, dev_n) + RECV_STATUS)
#define DEVICE_COMMAND(int_n, dev_n) (memaddr*)(DEV_REG(int_n, dev_n) + RECV_COMMAND)

#define TERMINAL_RECV_STATUS(int_n, dev_n) (memaddr*)(DEV_REG(int_n, dev_n) + RECV_STATUS)
#define TERMINAL_RECV_COMMAND(int_n, dev_n) (memaddr*)(DEV_REG(int_n, dev_n) + RECV_COMMAND)
#define TERMINAL_TRANSM_STATUS(int_n, dev_n) (memaddr*)(DEV_REG(int_n, dev_n) + TRANSM_STATUS)
#define TERMINAL_TRANSM_COMMAND(int_n, dev_n) (memaddr*)(DEV_REG(int_n, dev_n) + TRANSM_COMMAND)

/* Addresses for Inter-Processor Interrupts. */
#define IPI_INBOX ((memaddr*) 0x10000400)
#define IPI_OUTBOX ((memaddr*) 0x10000404)

/***** UTILS *****/

/*Macro che fa sostituisce la memcpy per gli stati*/
#define copyState(source, dest) ({\
	int i; for(i=0;i<29;i++)\
		(*dest).gpr[i]=(*source).gpr[i];\
	(*dest).entry_hi=(*source).entry_hi;\
	(*dest).cause=(*source).cause;\
	(*dest).status=(*source).status;\
	(*dest).pc_epc=(*source).pc_epc;\
	(*dest).hi=(*source).hi;\
	(*dest).lo=(*source).lo;\
})

/* Macros to send an IPI to the processor with specified PRID and Acknowledge an IPI */
#define SEND_IPI(i) *IPI_OUTBOX = (1 << (i + 8)) + 1
#define ACK_IPI *IPI_INBOX = 1

#endif


