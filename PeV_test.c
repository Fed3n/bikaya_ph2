/*****************************************************************************
 * Copyright 2019, Renzo Davoli, Mattia Maldini                              *
 * Copyright 2004, 2005 Michael Goldweber, Davide Brini.                     *
 *****************************************************************************/

/*********************************P2TEST.C*******************************
 *
 *	Test program for the Bikaya Kernel: phase 2.
 *
 *	Produces progress messages on Terminal0.
 *
 *	This is pretty convoluted code, so good luck!
 *
 *		Aborts as soon as an error is detected.
 *
 *      Modified by Michael Goldweber on May 15, 2004
 *      Modified by Davide Brini on Nov 26, 2004
 *      Modified by Renzo Davoli 2010
 *      Modified by Miro Mannino on May 8, 2010
 *      Modified by Mattia Maldini, Renzo Davoli 2020
 */

#ifdef TARGET_UMPS
#include "umps_include/libumps.h"
#include "umps_include/arch.h"
#include "umps_include/types.h"
#define FRAME_SIZE 4096
/* Elapsed clock ticks (CPU instructions executed) since system power on.
   Only the "low" part is actually used. */
#define BUS_TODLOW  0x1000001c
#define BUS_TODHIGH 0x10000018

#define VMON  0x07000000
#define VMOFF (~VMON)

//#define getTODLO() (*((unsigned int *)BUS_TODLOW))

#define SP(s)                    s.reg_sp
#define PC(s)                    s.pc_epc
#define VM(s)                    s.status
#define REG0(s)                  s.reg_a0
#define STATUS_ALL_INT_ENABLE(x) (x | (0xFF << 8))
#define CAUSE_EXCCODE_GET(cause) ((cause >> 2) & 0x1F)
#define CAUSE_CODE(s)            CAUSE_EXCCODE_GET(s.cause)

#endif

#ifdef TARGET_UARM
#include "uarm_include/libuarm.h"
#include "uarm_include/arch.h"
#include "uarm_include/uARMtypes.h"

#define VMON  0x00000001
#define VMOFF (~VMON)

#define CAUSE_CODE(s) CAUSE_EXCCODE_GET(s.CP15_Cause)
#define SP(s)         s.sp
#define PC(s)         s.pc
#define VM(s)         s.CP15_Control
#define REG0(s)       s.a1
#endif

#include "const_bikaya.h"
#include "types_bikaya.h"

typedef unsigned int devregtr;
typedef unsigned int cpu_t;
typedef unsigned int pid_t;

/* if these are not defined */
/* typedef U32 cpu_t; */
/* typedef U32 pid_t; */

/* hardware constants */
#define PRINTCHR 2
#define BYTELEN  8
#define RECVD    5
#define TRANSM   5

#define TERMSTATMASK 0xFF
#define TERMCHARMASK 0xFF00

#define MINLOOPTIME 1000
#define LOOPNUM     1000

#define BADADDR 0xFFFFFFFF /* could be 0x00000000 as well */

/* Software and other constants */
#define ERROR -1

/* just to be clear */
#define NOLEAVES 4 /* number of leaves of p7 process tree */
#define MAXSEM   20

int term_mut = 1,   /* for mutual exclusion on terminal */
    s[MAXSEM + 1],  /* semaphore array */
    testsem    = 0, /* for a simple test */
    startp2    = 0, /* used to start p2 */
    endp2      = 0, /* used to signal p2's demise */
    blkp3      = 1, /* used to block second incaration of p3 */
    synp3      = 0, /* used to allow p3 incarnations to synhronize */
    endp3      = 0, /* to signal demise of p3 */
    endp4      = 0, /* to signal demise of p4 */
    endp7      = 0, /* to signal demise of p7 */
    endcreate  = 0, /* for a p7 leaf to signal its creation */
    blkleaves  = 0, /* for a p7 leaf to signal its creation */
    blkp7      = 0, /* to block p7 */
    blkp7child = 0; /* to block p7's children */

state_t p2state, p3state, p4state, p5state, p6state;
state_t p7rootstate, child1state, child2state;
state_t gchild1state, gchild2state, gchild3state, gchild4state;

/* trap states for p4 */
state_t pstat_n, mstat_n, sstat_n, pstat_o, mstat_o, sstat_o;

int p1p2synch = 0; /* to check on p1/p2 synchronization */

int p7inc;     /* p7's incarnation number */
int p3inc = 1; /* p3 incarnation number */

int          creation      = 0; /* return code for SYSCALL invocation */
memaddr *    p4MemLocation = (memaddr *)0x34;
unsigned int p4Stack;

pid_t p3pid;
pid_t testpid;
pid_t p7pid;
pid_t leaf1pid, leaf2pid, leaf3pid, leaf4pid;

void p2(), p3(), p4(), p4a(), p4b(), p5(), p6(), p6a();
void p7root(), child1(), child2(), p7leaf();

unsigned int set_sp_pc_status(state_t *s, state_t *copy, unsigned int pc) {
    STST(s);

#ifdef TARGET_UMPS
    s->reg_sp = copy->reg_sp - FRAME_SIZE;
    s->pc_epc = pc;
    s->status = STATUS_ALL_INT_ENABLE(s->status);
    return s->reg_sp;
#endif

#ifdef TARGET_UARM
    s->sp   = copy->sp - FRAME_SIZE;
    s->pc   = pc;
    s->cpsr = STATUS_ALL_INT_ENABLE(s->cpsr);
    return s->sp;
#endif
}

//STAMPA

#ifdef TARGET_UMPS
static termreg_t *term0_reg = (termreg_t *)DEV_REG_ADDR(IL_TERMINAL, 0);

static unsigned int tx_status(termreg_t *tp) {
    return ((tp->transm_status) & TERM_STATUS_MASK);
}

void termprint(char *str) {
    while (*str) {
        unsigned int stat = tx_status(term0_reg);
        if (stat != ST_READY && stat != ST_TRANSMITTED)
            return;

        term0_reg->transm_command = (((*str) << CHAR_OFFSET) | CMD_TRANSMIT);

        while ((stat = tx_status(term0_reg)) == ST_BUSY)
            ;

        term0_reg->transm_command = CMD_ACK;

        if (stat != ST_TRANSMITTED)
            return;
        else
            str++;
    }
}
#endif
#ifdef TARGET_UARM
#define termprint(str) tprint(str);
#endif


//


#define print(msg) termprint(msg)

//int testsem = 0;


/*                                                                   */
/*                 p1 -- the root process                            */
/*                                                                   */
void test() {

	//int mmm = 0;
	if (startp2 == 0) print("almeno ");
	int* a = &startp2;
	if (*a == 0) print("aooooohhh");
	int* b = (int)&startp2;
	if (*b == 0) print("auuuuuhhh");
	print("\n");





    SYSCALL(VERHOGEN, (int)&testsem, 0, 0); /* V(testsem)   */
	
	if (testsem == 0 ) print("è zero"); 	

    if (testsem != 1) {
        print("error: p1 v(testsem) with no effects\n");
        PANIC();
    }

    print("p1 v(testsem)\n");

    /* set up states of the other processes */

    /* set up p2's state */
    set_sp_pc_status(&p2state, &p2state, (unsigned int)p2);

    /* create process p2 */
    SYSCALL(CREATEPROCESS, (int)&p2state, DEFAULT_PRIORITY, 0); /* start p2     */
    print("p2 was started\n");

    SYSCALL(VERHOGEN, (int)&startp2, 0, 0); /* V(startp2)   */

    /* P1 blocks until p2 finishes and Vs endp2 */
    SYSCALL(PASSEREN, (int)&endp2, 0, 0); /* P(endp2)     */
    print("p1 knows p2 ended\n");

    /* make sure we really blocked */
    if (p1p2synch == 0)
        print("error: p1/p2 synchronization bad\n");
  
}

/* p2 -- semaphore and cputime-SYS test process */
void p2() {
    int   i;                          /* just to waste time  */
    cpu_t now1, now2;                 /* times of day        */
    cpu_t user_t1, user_t2;           /* user time used       */
    cpu_t kernel_t1, kernel_t2;       /* kernel time used       */
    cpu_t wallclock_t1, wallclock_t2; /* wallclock time used       */

    /* startp2 is initialized to 0. p1 Vs it then waits for p2 termination */
    SYSCALL(PASSEREN, (int)&startp2, 0, 0); /* P(startp2)   */

    print("p2 starts\n");

    /* initialize all semaphores in the s[] array */
    for (i = 0; i <= MAXSEM; i++)
        s[i] = 0;

    /* V, then P, all of the semaphores in the s[] array */
    for (i = 0; i <= MAXSEM; i++) {
        SYSCALL(VERHOGEN, (int)&s[i], 0, 0); /* V(S[I]) */
        SYSCALL(PASSEREN, (int)&s[i], 0, 0); /* P(S[I]) */
        if (s[i] != 0)
            print("error: p2 bad v/p pairs\n");
    }

    print("p2 v/p pairs successfully\n");

    /* test of SYS6 */

    now1 = getTODLO();                                                       /* time of day   */
    SYSCALL(GETCPUTIME, (int)&user_t1, (int)&kernel_t1, (int)&wallclock_t1); /* CPU time used */

    int localsem = 0;
    /* delay for some time */
    for (i = 1; i < LOOPNUM; i++) {
        SYSCALL(VERHOGEN, (int)&localsem, 0, 0);
        SYSCALL(PASSEREN, (int)&localsem, 0, 0);
    }



    p1p2synch = 1; /* p1 will check this */

    SYSCALL(VERHOGEN, (int)&endp2, 0, 0); /* V(endp2)     */

    SYSCALL(TERMINATEPROCESS, 0, 0, 0); /* terminate p2 */

    /* just did a SYS2, so should not get to this point */
    print("error: p2 didn't terminate\n");
    PANIC(); /* PANIC! */
}

