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

extern pcb_t* currentProc;

void syscall_handler(){
	kernel_timer_update(currentProc);
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
				special_handler(0,p,arg1,arg2,arg3);
		}
	}
	else
		HALT();
	//valore di ritorno (se non è stato modificato)
	if (retvalue != 1) 
		p->ST_RET = retvalue;
	schedule();
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
	int i;
	while(line<=7 && !(INTERRUPT_LINE_CAUSE(getCAUSE(), line))) line++;
	/*Siccome il PLT non e’ presente su uARM, e’
	conveniente sfruttare l’interval timer su
	entrambe le piattaforme*/
	switch(line){
		case PROCESSOR_LOCAL_TIMER:
			interrupt12();
		case BUS_INTERVAL_TIMER:
			interrupt12();
		case DISK_DEVICES:
			/*controllo ogni bit dell'interrupt line per vedere quali
			device hanno un interrupt in sospeso*/
			for(i = 0; i < DEV_PER_INT; i++){
				unsigned int* bit_vec = (unsigned int*)INT_BIT_VEC(DISK_DEVICES);
				if(*bit_vec & (1<<i))
					devInterrupt(line,i);
			}
		case TYPE_DEVICES:
			/*controllo ogni bit dell'interrupt line per vedere quali
			device hanno un interrupt in sospeso*/
			for(i = 0; i < DEV_PER_INT; i++){
				unsigned int* bit_vec = (unsigned int*)INT_BIT_VEC(TYPE_DEVICES);
				if(*bit_vec & (1<<i))
					devInterrupt(line,i);
			}
		case NETWORK_DEVICES:
			/*controllo ogni bit dell'interrupt line per vedere quali
			device hanno un interrupt in sospeso*/
			for(i = 0; i < DEV_PER_INT; i++){
				unsigned int* bit_vec = (unsigned int*)INT_BIT_VEC(NETWORK_DEVICES);
				if(*bit_vec & (1<<i))
					devInterrupt(line,i);
			}
		case PRINTER_DEVICES:
			/*controllo ogni bit dell'interrupt line per vedere quali
			device hanno un interrupt in sospeso*/
			for(i = 0; i < DEV_PER_INT; i++){
				unsigned int* bit_vec = (unsigned int*)INT_BIT_VEC(PRINTER_DEVICES);
				if(*bit_vec & (1<<i))
					devInterrupt(line,i);
			}
		case TERMINAL_DEVICES:
			for(i = 0; i < DEV_PER_INT; i++){
				unsigned int* bit_vec = (unsigned int*)INT_BIT_VEC(TERMINAL_DEVICES);
				if(*bit_vec & (1<<i)){
					termInterrupt(i);
				}
			}
	}
	schedule();
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
	if (currentProc->excareas[type].used == 1){
		/*passo i parametri in caso sia una syscall*/
		if(type == 0){
			/*idk actually?*/
		}
		ownmemcpy(oldarea, currentProc->excareas[type].oldarea, sizeof(state_t));
		state_t* p = (currentProc->excareas[type].newarea);
		LDST(TO_LOAD(p));
	}
	else
		SYSCALL(3,0,0,0);
}
