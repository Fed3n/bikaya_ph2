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

#include "const_bikaya.h"
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
/*numero totale di devices, 5*8 + 8 (il terminale ha sia recv che transm)*/
#define TOT_DEV_N (N_EXT_IL*N_DEV_PER_IL + N_DEV_PER_IL)
/*ritorna il numero di semaforo della struttura devsem_t corrispondente al device*/
#define DEVSEM_N(reg) (((reg)-DEV_REG_START)/(DEV_REG_SIZE))
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
#define STATUS_ALL_INT_DISABLE_KM_LT(status) ((status) | (STATUS_IEp) | (STATUS_TE))
/*interrupt disabled eccetto i timer, kernel mode, virtual memory off*/
#define STATUS_ALL_INT_ENABLE_KM_LT(status) ((status) | (STATUS_IEp) | (STATUS_IM(1)) | (STATUS_IM(2)) | (STATUS_TE))
#define STATUS_ALL_INT_ENABLE_KM(status) ((status) | (STATUS_IEp) | (STATUS_IM_MASK) | (STATUS_TE))
#define STATUS_ENABLE_ALL_INT(status) ((status) | (0xFF << 8))

/*macro da passare a LDST*/
#define TO_LOAD(status) (status)

/*accesso ai registri dello state_t*/
#define ST_A0 reg_a0
#define ST_A1 reg_a1
#define ST_A2 reg_a2
#define ST_A3 reg_a3
/*accesso al registro in cui inserire il valore di ritorno*/
#define ST_RET reg_v0
/*macro per accedere al pc da state_t*/
#define ST_PC pc_epc

/*macro per controllare la linea corrispondente dell'interrupt*/
#define INTERRUPT_LINE_CAUSE(cause, line) ((cause) & CAUSE_IP(line))
/*macro per ottenere interrupting device map register*/
#define INT_BIT_VEC(line) (0x1000003C + (EXT_IL_INDEX(line) * 4))
/*la seguente macro ritorna il valore del bit cause prendendo in input lo state_t corrispondente*/
#define CAUSE_REG(area) CAUSE_GET_EXCCODE(area->cause)

/*workaround per non causare redefinizioni a causa di def gia' presente nel test*/
#define BUS_TODLOW 0x1000001c
/*CPU Ticks trascorsi, la macro era giá presente per uarm ma mancante per umps*/
#define getTODLO() (*((unsigned int *)BUS_TODLOW))

/*word count da modificare prima di chiamare LDST dopo un'eccezione*/
#define SYSBP_PC 1
#define INT_PC 0

#define SYSCALL_EXC EXC_SYS

#endif


#ifdef TARGET_UARM

#define STATUS_ENABLE_ALL_INT(status) STATUS_ALL_INT_ENABLE(status)
/*macro per usare LDST su uarm*/
#define TO_LOAD(status) &(status->a1)

/*accesso ai registri dello state_t*/
#define ST_A0 a1
#define ST_A1 a2
#define ST_A2 a3
#define ST_A3 a4
/*accesso al registro in cui inserire il valore di ritorno*/
#define ST_RET a1
/*macro per accedere al pc da state_t*/
#define ST_PC pc

/*macro per controllare la linea corrispondente dell'interrupt*/
#define INTERRUPT_LINE_CAUSE(cause, line) CAUSE_IP_GET(cause, line)
/*macro per ottenere interrupting device map register*/
#define INT_BIT_VEC(line) INT_DEV_VECTOR(line)
/*la seguente macro ritorna il valore del bit cause prendendo in input lo state_t corrispondente*/
#define CAUSE_REG(area) CAUSE_EXCCODE_GET(area->CP15_Cause)

/*word count da modificare prima di chiamare LDST dopo un'eccezione*/
#define SYSBP_PC 0
#define INT_PC (-1)

#define SYSCALL_EXC EXC_SYSCALL

#endif

#endif
