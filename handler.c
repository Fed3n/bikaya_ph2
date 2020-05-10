#include "handler.h"
#include "scheduler.h"
#include "auxfun.h"

#ifdef TARGET_UMPS
extern void termprint(char *str);
#endif

#ifdef TARGET_UARM
#define termprint(str) tprint(str);
#endif

extern pcb_t* currentProc;

void user_time_update() {
	if(currentProc != NULL) {
		currentProc->start_user_time = currentProc->start_user_time + getTODLO() - currentProc->total_user_time;
		currentProc->start_user_time = 0;
	}	
}
void kernel_time_update() {
	if(currentProc != NULL) {
		currentProc->start_kernel_time = currentProc->start_kernel_time + getTODLO() - currentProc->total_kernel_time;
		currentProc->start_kernel_time = 0;
	}
}

void syscall_handler(){
	user_time_update();
	kernel_time_update();
	currentProc->start_kernel_time = getTODLO();
	/*recupero dell'old area*/
	state_t* p = (state_t*)SYSBK_OLDAREA;
	p->ST_PC = p->ST_PC + INT_PC*WORDSIZE;
	ownmemcpy(p, &(currentProc->p_s), sizeof(state_t));
	/*controllo se l'eccezione sollevata è una system call*/
	if (CAUSE_REG(p) == SYSCALL_EXC) {
		/*recupero del tipo e dei parametri della systemcall*/
		unsigned int sysNum = p->ST_A0;
		unsigned int arg1 = p->ST_A1;
		unsigned int arg2 = p->ST_A2;
		unsigned int arg3 = p->ST_A3;

		switch (sysNum){
			case 1: 
				get_CPU_time((unsigned int*)arg1,(unsigned int*)arg2,(unsigned int*)arg3);
				break;
			case 3:
				sys3();
			break;
			default:
				termprint("Syscall not yet managed.\n");
				HALT();
		}
	}
	else{
		termprint("BREAKPOINT!\n");
		HALT();
	}
	currentProc->start_user_time = getTODLO();
}

void interrupt_handler(){
	user_time_update();
	/*Se c'è un processo in corso che è stato interrotto*/
	if(currentProc != NULL){
		/*PC da decrementare di 1 word su uarm, niente su umps*/
		state_t* p = (state_t *)INT_OLDAREA;
		p->ST_PC = p->ST_PC + INT_PC*WORDSIZE;
		ownmemcpy(p, &(currentProc->p_s), sizeof(state_t));
	}
	int line = 0;
	while(line<=7 && !(INTERRUPT_LINE_CAUSE(getCAUSE(), line))) line++;
	/*Siccome il PLT non e’ presente su uARM, e’
	conveniente sfruttare l’interval timer su
	entrambe le piattaforme*/
	switch(line){
		case PROCESSOR_LOCAL_TIMER:
			interrupt12();
		case BUS_INTERVAL_TIMER:
			interrupt12();
		default:
			termprint("Interrupt line not yet managed.\n");
			HALT();
	}
	/*Usciti da un interrupt si torna in user mode*/
	if(currentProc != NULL) currentProc->start_user_time = getTODLO();	
	schedule();
}

void tlb_handler(){
	termprint("TLB!");
	HALT();
}

void trap_handler(){
	termprint("TRAP!");
	HALT();
}
