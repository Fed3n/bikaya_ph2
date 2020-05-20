#include "init.h"
#include "syscall.h"
#include "handler.h"
#include "auxfun.h"

extern int devsem[TOT_DEV_N];
extern int waitIOsem[TOT_DEV_N];

#ifdef TARGET_UMPS
/*inizializza l'exception area in kernel mode, interrupt disabilitati*/
void initExcarea(state_t* p, void* handler){
	ownmemset(p, 0, sizeof(state_t));
	p->reg_sp = RAMTOP;
	p->pc_epc = (memaddr)handler;
	p->status = STATUS_ALL_INT_DISABLE_KM_LT(p->status);
}

/*inizializza processo in kernel mode, interrupt abilitati*/
/*n = numero del processo, che influenza la priorità e dove gli viene assegnato spazio in ram*/
void initProcess_KM(pcb_t* p, void* fun, int n){
	p->p_s.reg_sp = (RAMTOP-(RAM_FRAMESIZE*n));
	p->p_s.pc_epc = (memaddr)fun;
	p->p_s.status = STATUS_ALL_INT_ENABLE_KM(p->p_s.status);
	p->original_priority = n;
	p->priority = n;
}

/*inizializza processo in kernel mode, interrupt disabilitati escluso timer*/
/*n = numero del processo, che influenza la priorità e dove gli viene assegnato spazio in ram*/
void initProcess_KM_noINT(pcb_t* p, void* fun, int n){
	p->p_s.reg_sp = (RAMTOP-(RAM_FRAMESIZE*n));
	p->p_s.pc_epc = (memaddr)fun;
	p->p_s.status = STATUS_ALL_INT_ENABLE_KM_LT(p->p_s.status);
	p->original_priority = n;
	p->priority = n;
}
#endif

#ifdef TARGET_UARM
/*inizializza l'exception area in kernel mode, interrupt disabilitati*/
void initExcarea(state_t* p, void* handler){
	ownmemset(p, 0, sizeof(state_t));
	p->pc = (memaddr)handler;
	p->sp = RAMTOP;
	p->cpsr = (p->cpsr | STATUS_SYS_MODE);
	p->cpsr = STATUS_ALL_INT_DISABLE(p->cpsr);
	p->CP15_Control = CP15_DISABLE_VM(p->CP15_Control);
}

/*inizializza processo in kernel mode, interrupt abilitati*/
/*n = numero del processo, che influenza la priorità e dove gli viene assegnato spazio in ram*/
void initProcess_KM(pcb_t* p, void* fun, int n){
	p->p_s.pc = (memaddr)fun;
	p->p_s.sp = (RAMTOP-(RAM_FRAMESIZE*n));
	p->p_s.cpsr = (p->p_s.cpsr | STATUS_SYS_MODE);
	p->p_s.cpsr = STATUS_ALL_INT_ENABLE(p->p_s.cpsr);
	p->p_s.CP15_Control = CP15_DISABLE_VM(p->p_s.CP15_Control);
	p->original_priority = n;
	p->priority = n;
}

/*inizializza processo in kernel mode, interrupt disabilitati escluso timer*/
/*n = numero del processo, che influenza la priorità e dove gli viene assegnato spazio in ram*/
void initProcess_KM_noINT(pcb_t* p, void* fun, int n){
	p->p_s.pc = (memaddr)fun;
	p->p_s.sp = (RAMTOP-(RAM_FRAMESIZE*n));
	p->p_s.cpsr = (p->p_s.cpsr | STATUS_SYS_MODE);
	p->p_s.cpsr = STATUS_DISABLE_INT(p->p_s.cpsr);
	p->p_s.cpsr = STATUS_ENABLE_TIMER(p->p_s.cpsr);
	p->p_s.CP15_Control = CP15_DISABLE_VM(p->p_s.CP15_Control);
	p->original_priority = n;
	p->priority = n;
}
#endif

/*Inizializza ogni exception area e le assegna una handler function*/
void initAreas(){
/*AREA INTERRUPT*/
initExcarea((state_t *)INT_NEWAREA, interrupt_handler);

/*AREA TLB*/
initExcarea((state_t *)TLB_NEWAREA, tlb_handler);

/*AREA PGMTRAP*/
initExcarea((state_t *)PGMTRAP_NEWAREA, trap_handler);

/*AREA SYSKB*/
initExcarea((state_t *)SYSBK_NEWAREA, syscall_handler);
}

/*Inizializza strutture ausiliarie della fase2, ma in realtà ci sono solo i
vettori dei semafori dei device e di attesaIO che sono inizializzati a 0 e 1*/
void initStructs(){
	ownmemset(&devsem,0,TOT_DEV_N*sizeof(int));
	ownmemset(&waitIOsem,1,TOT_DEV_N*sizeof(int));
}
