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
	/*recupero dell'old area*/
	state_t* p = (state_t*)SYSBK_OLDAREA;
	p->ST_PC = p->ST_PC + SYSBP_PC*WORDSIZE;
	ownmemcpy(p, &(currentProc->p_s), sizeof(state_t));
	int retvalue = 1; //i valori possibili sono 0 e -1, capisco così se è stato modificato
	/*controllo se l'eccezione sollevata è una system call*/
	if (CAUSE_REG(p) == SYSCALL_EXC) {
		/*recupero del tipo e dei parametri della systemcall*/
		unsigned int sysNum = p->ST_A0;
		unsigned int arg1 = p->ST_A1;
		unsigned int arg2 = p->ST_A2;
		unsigned int arg3 = p->ST_A3;
		
/*		int arg1 = 0;

		int* prov = (int*)p->ST_A1;
		if ((*prov) == 0) termprint("ella");
		if ((*prov) >= 2) termprint("elli");
		if ((*prov) <= 200000000) termprint("elle");
*/
		switch (sysNum){
			case CREATE_PROC:
				retvalue = createProcess((state_t*)arg1,(int)arg2,(void**)arg3);
			case TERMINATE_PROC:
				retvalue = terminateProcess((int*)arg1);
			break;
			case VERHOGEN:
				verhogen((int*)arg1);			
			break;
			case PASSEREN:
				passeren((int*)arg1);
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
	//valore di ritorno (se non è stato modificato)
	if (retvalue != 1) 
		p->ST_RET = retvalue;
	schedule();
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
	termprint("TLB!");
	HALT();
}

void trap_handler(){
	termprint("TRAP!");
	HALT();
}
