/**************************************************************************** 
 *
 * This header file contains the global customized constant & macro definitions.
 * 
 ****************************************************************************/

#include "base.h"

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
#define TERMINAL_SEND_ACK(int_n, dev_n) (memaddr*)(DEV_REG(int_n, dev_n) + TRANSM_COMMAND)




