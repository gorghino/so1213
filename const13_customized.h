#ifndef CONST_H
#define CONST_H

/**************************************************************************** 
 *
 * This header file contains the global customized constant & macro definitions.
 * 
 ****************************************************************************/

#include "base.h"

#define	MAX_CPUS 4

#define STOPPED 0
#define RUNNING 1

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

/* Devices registers addresses  */
#define DEV_REG(int_n, dev_n) (DEV_REGS_START + ((int_n - 3) * 0x80) + (dev_n * 0x10))
#define TERMINAL_RECV_STATUS(int_n, dev_n) (memaddr*)(DEV_REG(int_n, dev_n) + RECV_STATUS)
#define TERMINAL_RECV_COMMAND(int_n, dev_n) (memaddr*)(DEV_REG(int_n, dev_n) + RECV_COMMAND)
#define TERMINAL_TRANSM_STATUS(int_n, dev_n) (memaddr*)(DEV_REG(int_n, dev_n) + TRANSM_STATUS)
#define TERMINAL_TRANSM_COMMAND(int_n, dev_n) (memaddr*)(DEV_REG(int_n, dev_n) + TRANSM_COMMAND)

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



/* Addresses for Inter-Processor Interrupts. */
#define IPI_INBOX ((memaddr*) 0x10000400)
#define IPI_OUTBOX ((memaddr*) 0x10000404)

/* Macros to send an IPI to the processor with specified PRID and Acknowledge an IPI */
#define SEND_IPI(i) *IPI_OUTBOX = (1 << (i + 8)) + 1
#define ACK_IPI *IPI_INBOX = 1

#define semPV 0
#define semClock 1
#define semScheduler 2

#endif


