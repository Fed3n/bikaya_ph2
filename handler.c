#include "handler.h"
#include "scheduler.h"
#include "auxfun.h"

/*system call numbers*/
#define GET_CPU_TIME 1
#define CREATE_PROC 2
#define TERMINATE_PROC 3
#define VERHOGEN 4 
#define PASSEREN 5
#define IO_COMMAND 6
#define SPEC_PASSUP 7
#define GET_PID 8

#define TYPE_SYS 0
#define TYPE_TLB 1
#define TYPE_PGMTRAP 2

extern pcb_t* currentProc;

void syscall_handler(){
	/*L'exception avviene in modalitá privilegiata, quindi si entra in Kernel Mode*/
	start_kernel_mode(currentProc);
	/*recupero dell'old area*/
	state_t* p = (state_t*)SYSBK_OLDAREA;
	/*aggiornamento PC*/
	p->ST_PC = p->ST_PC + SYSBP_PC*WORDSIZE;
	ownmemcpy(p, &(currentProc->p_s), sizeof(state_t));
	int retvalue = 1; //i valori possibili sono 0 e -1, capisco così se è stato modificato
	/*controllo se l'eccezione sollevata è una system call*/
	if (CAUSE_REG(p) == SYSCALL_EXC) {
		/*SYSCALL*/
		/*recupero del tipo e dei parametri della systemcall*/
		unsigned int sysNum = p->ST_A0;
		unsigned int arg1 = p->ST_A1;
		unsigned int arg2 = p->ST_A2;
		unsigned int arg3 = p->ST_A3;

		switch (sysNum){
			case GET_CPU_TIME:
				get_cpu_time((unsigned int*)arg1,(unsigned int*)arg2,(unsigned int*)arg3);
				break;
			case CREATE_PROC:
				retvalue = createProcess((state_t*)arg1,(int)arg2,(void**)arg3);
				break;
			case TERMINATE_PROC:
				retvalue = terminateProcess((void*)arg1);
				break;
			case VERHOGEN:
				verhogen((int*)arg1);		
				break;
			case PASSEREN:
				passeren((int*)arg1);
				break;
			case IO_COMMAND:
				do_IO((unsigned int)arg1,(unsigned int*)arg2, (int)arg3);
				break;
			case SPEC_PASSUP:
				retvalue = spec_passup((int)arg1,(state_t*)arg2,(state_t*)arg3);
				break;
			case GET_PID:
				get_pid_ppid((void**)arg1,(void**)arg2);
				break;
			default:
				special_handler(TYPE_SYS,p);
		}
		/*valore di ritorno (se non è stato modificato)*/
		if (retvalue != 1) 
			p->ST_RET = retvalue;
		schedule();
	}
	else
		/*BREAKPOINT*/
		special_handler(TYPE_SYS,p);
}

void interrupt_handler(){
	/*L'exception avviene in modalitá privilegiata, quindi si entra in Kernel Mode*/
	start_kernel_mode(currentProc);
	/*Se c'è un processo in corso che è stato interrotto*/
	if(currentProc != NULL){
		/*PC da decrementare di 1 word su uarm, niente su umps*/
		state_t* p = (state_t *)INT_OLDAREA;
		p->ST_PC = p->ST_PC + INT_PC*WORDSIZE;
		ownmemcpy(p, &(currentProc->p_s), sizeof(state_t));
	}
	int line;
	int i;
	for(line=0; line < 8; line++){
		if(INTERRUPT_LINE_CAUSE(getCAUSE(), line)){
			/*Siccome il PLT non e’ presente su uARM, e’
            conveniente sfruttare l’interval timer su
            entrambe le piattaforme*/
			if(line == PROCESSOR_LOCAL_TIMER || line == BUS_INTERVAL_TIMER)
				timerInterrupt();
			else if(line == DISK_DEVICES || line == TYPE_DEVICES ||
					line == NETWORK_DEVICES || line == PRINTER_DEVICES){
				/*controllo ogni bit dell'interrupt line per vedere quali
				device hanno un interrupt in sospeso*/
				unsigned int* bit_vec = (unsigned int*)INT_BIT_VEC(line);
				for(i = 0; i < DEV_PER_INT; i++){
					if(*bit_vec & (1<<i))
						devInterrupt(line,i);
				}
			}
			else if(line == TERMINAL_DEVICES){
				/*controllo ogni bit dell'interrupt line per vedere quali
				device hanno un interrupt in sospeso*/
				unsigned int* bit_vec = (unsigned int*)INT_BIT_VEC(TERMINAL_DEVICES);
				for(i = 0; i < DEV_PER_INT; i++){
					if(*bit_vec & (1<<i)){
						termInterrupt(i);
					}
				}
			}
		}
	}
	schedule();
}

/*TLB e PGMTrap handler gestiti attraverso handler speciali specificati da
specpassup (se specificati, altrimenti terminazione)*/

void tlb_handler(){
	/*L'exception avviene in modalitá privilegiata, quindi si entra in Kernel Mode*/
	start_kernel_mode(currentProc);
	state_t* old = (state_t *)TLB_OLDAREA;
	special_handler(TYPE_TLB,old);
}

void trap_handler(){
	/*L'exception avviene in modalitá privilegiata, quindi si entra in Kernel Mode*/
	start_kernel_mode(currentProc);
	state_t* old = (state_t *)PGMTRAP_OLDAREA;
	special_handler(TYPE_PGMTRAP,old);
}

void special_handler(int type, state_t* oldarea){
	if (currentProc->excareas[type].used == 1){
		/*in caso di syscall con parametri, questi devono essere recuperati dall'oldarea*/
		ownmemcpy(oldarea, currentProc->excareas[type].oldarea, sizeof(state_t));
		state_t* p = currentProc->excareas[type].newarea;
		LDST(TO_LOAD(p));
	}
	else
		/*Se l'eccezione speciale non è gestita, terminazione*/
		SYSCALL(TERMINATE_PROC,0,0,0);
}
