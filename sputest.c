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

state_t spunew, spuold;

void systest(){
    termprint("Nice!");
    SYSCALL(3,0,0,0);
}

void traptest(){

}

void loopingproc(){
    for(;;)
        termprint("looping!\n");
        WAIT();
}

void spufailtest(){
    termprint("Hi I'm spufailtest!\n");
    SYSCALL(9,0,0,0);
    termprint("This shouldn't be printed\n");
}

void sputest(){
    termprint("Hi I'm sputest!\n");
    initExcarea(&spunew, systest);
    SYSCALL(7,0,&spuold,&spunew);
    termprint("Sys7 done!");
    SYSCALL(9,0,0,0);
}

/*Inizializza le exception area, i PCB, mette i processi in ready queue, setta timer e poi chiama lo scheduler*/
int main(){
	initAreas();
	termprint("AREA DONE!\n");

	initPcbs();
	termprint("PCB DONE!\n");

	initASL();
	termprint("ASL DONE!\n");

    initSysData();

    pcb_t* a = allocPcb();
    pcb_t* b = allocPcb();
    pcb_t* c = allocPcb();

    initProcess_KM(a,loopingproc,1);
    initProcess_KM(b,sputest,2);
    initProcess_KM(c,spufailtest,3);
    initReadyQueue();
    insertReadyQueue(a);
    insertReadyQueue(b);
    insertReadyQueue(c);

    termprint("Calling schedule...\n");
    schedule();
}