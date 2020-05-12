#include "handler.h"
#include "scheduler.h"
#include "auxfun.h"

#ifdef TARGET_UMPS
extern void termprint(char *str);
#endif

#ifdef TARGET_UARM
#define termprint(str) tprint(str);
#endif

/*system call numbers*/
#define GET_CPU_TIME 1
#define CREATE_PROC 2
#define TERMINATE_PROC 3
#define VERHOGEN 4
#define PASSEREN 5
#define IO_COMMAND 6
#define SPEC_PASSUP 7
#define GET_PID 8

extern pcb_t* currentProc;

void syscall_handler(){
	kernel_timer_update(currentProc);
	/*recupero dell'old area*/
	state_t* p = (state_t*)SYSBK_OLDAREA;
	p->ST_PC = p->ST_PC + SYSBP_PC*WORDSIZE;
	ownmemcpy(p, &(currentProc->p_s), sizeof(state_t));
	/*controllo se l'eccezione sollevata è una system call*/
	if (CAUSE_REG(p) == SYSCALL_EXC) {
		/*recupero del tipo e dei parametri della systemcall*/
		unsigned int sysNum = p->ST_A0;
		unsigned int arg1 = p->ST_A1;
		unsigned int arg2 = p->ST_A2;
		unsigned int arg3 = p->ST_A3;

		switch (sysNum){
			case GET_CPU_TIME:
				get_cpu_time((unsigned int*)arg1, (unsigned int*)arg2, (unsigned int*)arg3);
				break;
			case CREATE_PROC:
				createProcess((state_t*)arg1,(int)arg2,(void**)arg3);
			case TERMINATE_PROC:
				terminateProcess((int*)arg1);
			break;
			case VERHOGEN:
				verhogen((int*)arg1);			
			break;
			case PASSEREN:
				passeren((int*)arg1);
			break;
			case SPEC_PASSUP:
				spec_passup((int)arg1,(state_t*)arg2,(state_t*)arg3);
				break;
			default:
				special_handler(0,p,arg1,arg2,arg3);
		}
	}
	else{
		termprint("BREAKPOINT!\n");
		HALT();
	}
}

void interrupt_handler(){
	kernel_timer_update(currentProc);
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
}

void tlb_handler(){
	state_t* p = (state_t *)TLB_OLDAREA;
	special_handler(1,p,0,0,0);
}

void trap_handler(){
	termprint("Trap handler called\n");
	state_t* p = (state_t *)PGMTRAP_OLDAREA;
	special_handler(2,p,0,0,0);
}

void special_handler(int type, state_t* oldarea, unsigned int arg1, unsigned int arg2, unsigned int arg3){
	termprint("Entering special handler...\n");
	if (currentProc->excareas[type].used == 1){
		/*passo i parametri in caso sia una syscall*/
		if(type == 0){
			/*idk actually?*/
		}
		ownmemcpy(oldarea, currentProc->excareas[type].oldarea, sizeof(state_t));
		state_t* p = (currentProc->excareas[type].newarea);
		LDST(TO_LOAD(p));
	}
	else{
		termprint("Tipo speciale non definito.\n");
		SYSCALL(3,0,0,0);
	}
}
