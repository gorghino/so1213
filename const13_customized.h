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

/* Devices registers addresses  */
#define DEV_REG(i) (DEV_REGS_START + DEV_REG_SIZE * (i - 1))
