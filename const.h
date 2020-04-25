#ifndef _CONST_H
#define _CONST_H

#ifdef TARGET_UMPS
#include "libumps.h"
#include "arch.h"
#include "cp0.h"
#include "exc_area.h"
#endif

#ifdef TARGET_UARM
#include "libuarm.h"
#include "arch.h"
#endif

#include "types_bikaya.h"

/**************************************************************************** 
 *
 * This header file contains the global constant & macro definitions.
 * 
 ****************************************************************************/

/* Maxi number of overall (eg, system, daemons, user) concurrent processes */
#define MAXPROC 20

#define UPROCMAX 3  /* number of usermode processes (not including master proc
		       and system daemons */

#define	HIDDEN static
#define	TRUE 	1
#define	FALSE	0
#define ON 		1
#define OFF 	0
#define EOS '\0'

#define DEV_PER_INT 8 /* Maximum number of devices per interrupt line */

#define CR 0x0a   /* carriage return as returned by the terminal */

#define RAMBASE    *((unsigned int *)BUS_REG_RAM_BASE)
#define RAMSIZE    *((unsigned int *)BUS_REG_RAM_SIZE)
#define RAMTOP     (RAMBASE + RAMSIZE)
#define RAM_FRAMESIZE 4096
#define WORDSIZE 4

#define TIME_SLICE 3000
#define ACK_SLICE TIME_SLICE*(*(memaddr *)BUS_REG_TIME_SCALE)
#define INTER_PROCESSOR_INTERRUPT 0
#define PROCESSOR_LOCAL_TIMER 1
#define BUS_INTERVAL_TIMER 2
#define DISK_DEVICES 3
#define TYPE_DEVICES 4
#define NETWORK_DEVICES 5
#define PRINTER_DEVICES 6
#define TERMINAL_DEVICES 7

/**************************************************************
*
* UARM and UMPS specific global constants & macro definitions
*
**************************************************************/
#ifdef TARGET_UMPS
/*interrupt disabled, kernel mode, local timer on, virtual memory off*/
#define STATUS_ALL_INT_DISABLE_KM_LT(status) ((status) | (STATUS_TE))
/*interrupt disabled eccetto i timer, kernel mode, virtual memory off*/
#define STATUS_ALL_INT_ENABLE_KM_LT(status) ((status) | (STATUS_IEp) | (STATUS_IM(1)) | (STATUS_IM(2)) | (STATUS_TE))

/*macro da passare a LDST*/
#define TO_LOAD(status) (status)

/*accesso ai registri dello state_t*/
#define ST_A0 reg_a0
#define ST_A1 reg_a1
#define ST_A2 reg_a2
#define ST_A3 reg_a3
/*macro per accedere al pc da state_t*/
#define ST_PC pc_epc

/*macro per controllare la linea corrispondente dell'interrupt*/
#define INTERRUPT_LINE_CAUSE(cause, line) ((cause) & CAUSE_IP(line))
/*la seguente macro ritorna il valore del bit cause prendendo in input lo state_t corrispondente*/
#define CAUSE_REG(area) CAUSE_GET_EXCCODE(area->cause)

/*word count da modificare prima di chiamare LDST dopo un'eccezione*/
#define SYSBP_PC 1
#define INT_PC 0

#define SYSCALL_EXC EXC_SYS

#endif


#ifdef TARGET_UARM
/*macro per usare LDST su uarm*/
#define TO_LOAD(status) &(status->a1)

/*accesso ai registri dello state_t*/
#define ST_A0 a1
#define ST_A1 a2
#define ST_A2 a3
#define ST_A3 a4
/*macro per accedere al pc da state_t*/
#define ST_PC pc

/*macro per controllare la linea corrispondente dell'interrupt*/
#define INTERRUPT_LINE_CAUSE(cause, line) CAUSE_IP_GET(cause, line)
/*la seguente macro ritorna il valore del bit cause prendendo in input lo state_t corrispondente*/
#define CAUSE_REG(area) CAUSE_EXCCODE_GET(area->CP15_Cause)

/*word count da modificare prima di chiamare LDST dopo un'eccezione*/
#define SYSBP_PC 0
#define INT_PC (-1)

#define SYSCALL_EXC EXC_SYSCALL

#endif

#endif