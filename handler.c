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
extern excarea_t excareas[3];

void syscall_handler(){
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
			case 3:
				sys3();
				break;
			case 7:
				sys7((int)arg1,(state_t*)arg2,(state_t*)arg3);
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
	state_t* p = (state_t *)PGMTRAP_OLDAREA;
	special_handler(2,p,0,0,0);
}

void special_handler(int type, state_t* oldarea, unsigned int arg1, unsigned int arg2, unsigned int arg3){
	termprint("Entering special handler...\n");
	if (excareas[type].used == 1){
		/*passo i parametri in caso sia una syscall*/
		if(type == 0){
			/*idk actually?*/
		}
		ownmemcpy(oldarea, excareas[type].oldarea, sizeof(state_t));
		state_t* p = (excareas[type].newarea);
		LDST(TO_LOAD(p));
	}
	else{
		termprint("Tipo speciale non definito.\n");
		SYSCALL(3,0,0,0);
	}
}