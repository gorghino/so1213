#include "const13.h"
#include "uMPStypes.h"
#include "types13.h"
#include "libumps.h"
#include "arch.h"

#define TRANSMITTED	5
#define TRANSTATUS    2
#define ACK	1
#define PRINTCHR	2
#define CHAROFFSET	8
#define STATUSMASK	0xFF
#define	TERM0ADDR	0x10000250
#define DEVREGSIZE 16
#define READY     1
#define DEVREGLEN   4
#define TRANCOMMAND   3
#define BUSY      3


#define ST_READY           1
#define ST_BUSY            3
#define ST_TRANSMITTED     5
 
#define CMD_ACK            1
#define CMD_TRANSMIT       2
 
#define CHAR_OFFSET        8
#define TERM_STATUS_MASK   0xFF

char okbuf[2048];			/* sequence of progress messages */
char errbuf[128];			/* contains reason for failing */
char msgbuf[128];			/* nonrecoverable error message before shut down */

char *mp = okbuf;
typedef unsigned int devreg;

typedef unsigned int u32;
 
static int term_putchar(char c);
static u32 tx_status(termreg_t *tp);
 
static termreg_t *term0_reg = (termreg_t *) DEV_REG_ADDR(IL_TERMINAL, 0);


/******************************************************************************
 * I/O Routines to write on a terminal
 ******************************************************************************/

/* This function returns the terminal transmitter status value given its address */
devreg termstat(memaddr *stataddr) {
	return((*stataddr) & STATUSMASK);
}

/* This function prints a string on specified terminal and returns TRUE if
 * print was successful, FALSE if not   */
unsigned int termprint(char * str, unsigned int term) {

	memaddr *statusp;
	memaddr *commandp;

	devreg stat;
	devreg cmd;

	unsigned int error = FALSE;

	if (term < DEV_PER_INT) {
		/* terminal is correct */
		/* compute device register field addresses */
		statusp = (devreg *) (TERM0ADDR + (term * DEVREGSIZE) + (TRANSTATUS * DEVREGLEN));
		commandp = (devreg *) (TERM0ADDR + (term * DEVREGSIZE) + (TRANCOMMAND * DEVREGLEN));

		/* test device status */
		stat = termstat(statusp);
		if ((stat == READY) || (stat == TRANSMITTED)) {
			/* device is available */

			/* print cycle */
			while ((*str != '\0') && (!error)) {
				cmd = (*str << CHAROFFSET) | PRINTCHR;
				*commandp = cmd;

				/* busy waiting */
				while ((stat = termstat(statusp)) == BUSY);

				/* end of wait */
				if (stat != TRANSMITTED) {
					error = TRUE;
				} else {
					/* move to next char */
					str++;
				}
			}
		}	else {
			/* device is not available */
			error = TRUE;
		}
	}	else {
		/* wrong terminal device number */
		error = TRUE;
	}

	return (!error);
}


/* This function places the specified character string in okbuf and
 *	causes the string to be written out to terminal0 */
void addokbuf(char *strp) {

	termprint(strp, 0);
}

/* This function places the specified character string in errbuf and
 *	causes the string to be written out to terminal0.  After this is done
 *	the system shuts down with a panic message */
void adderrbuf(char *strp) {

	termprint(strp, 0);

	PANIC();
}

void increment_counter(struct pcb_t *pcb, void* pt)
{
	int *counter=pt;
	(*counter)++;
}


void term_puts(const char *str)
{
    while (*str)
        if (term_putchar(*str++))
            return;
}
 
int term_putchar(char c)
{
    u32 stat;
 
    stat = tx_status(term0_reg);
    if (stat != ST_READY && stat != ST_TRANSMITTED)
        return -1;
 
    term0_reg->transm_command = ((c << CHAR_OFFSET) | CMD_TRANSMIT);
 
    while ((stat = tx_status(term0_reg)) == ST_BUSY)
        ;
 
    term0_reg->transm_command = CMD_ACK;
 
    if (stat != ST_TRANSMITTED)
        return -1;
    else
        return 0;
}
 
static u32 tx_status(termreg_t *tp)
{
    return ((tp->transm_status) & TERM_STATUS_MASK);
}
