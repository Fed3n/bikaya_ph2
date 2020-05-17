#include "const.h"
#include "pcb.h"
#include "asl.h"
#include "auxfun.h"
#include "scheduler.h"
#include "handler.h"
#include "init.h"

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

memaddr* memLocation = (memaddr *)0x34;

state_t sys9new, sys9old, trapnew, trapold;

void systest(){
    termprint("systest! Nice!\n");
    schedule();
}

void traptest(){
    termprint("traptest success! Nice!\n");
    SYSCALL(3,0,0,0);
}

void spufailtest(){
    termprint("Hi I'm spufailtest!\n");
    SYSCALL(9,0,0,0);
    termprint("This shouldn't be printed\n");
    HALT();
}

void spufailtest2(){
    termprint("Hi I'm spufaultest2!\n");
    SYSCALL(7,0,&sys9old,&sys9new);
    termprint("Trying to call sys7 one time too many...\n");
    SYSCALL(7,0,&sys9old,&sys9new);
    termprint("This definitely shouldn't be printed.\n");
}

void sputest(){
    termprint("Hi I'm sputest!\n");
    initExcarea(&sys9new, systest);
    initExcarea(&trapnew, traptest);
    SYSCALL(7,0,&sys9old,&sys9new);
    termprint("Sys7 done!");
    SYSCALL(9,0,0,0);
    SYSCALL(7,2,&trapold,&trapnew);
    termprint("Now trying to cause a Trap...\n");
    *memLocation = *memLocation + 1;
    termprint("This shouldn't be printed either.\n");
    HALT();
}

/*Inizializza le exception area, i PCB, mette i processi in ready queue, setta timer e poi chiama lo scheduler*/
int main(){
	initAreas();
	termprint("AREA DONE!\n");

	initPcbs();
	termprint("PCB DONE!\n");

	initASL();
	termprint("ASL DONE!\n");

    pcb_t* a = allocPcb();
    pcb_t* b = allocPcb();
    pcb_t* c = allocPcb();

    initProcess_KM(a,spufailtest2,1);
    initProcess_KM(b,sputest,2);
    initProcess_KM(c,spufailtest,3);
    initReadyQueue();
    insertReadyQueue(a);
    insertReadyQueue(b);
    insertReadyQueue(c);

    //setTIMER(ACK_SLICE);

    termprint("Calling schedule...\n");
    schedule();
}