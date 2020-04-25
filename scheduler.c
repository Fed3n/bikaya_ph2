#include "pcb.h"
#include "scheduler.h"
#include "auxfun.h"

/*la testa della ready queue è hidden e vi ci si interfaccia tramite le funzioni dello scheduler*/
HIDDEN LIST_HEAD(readyQueue_h);

/*Puntatore a processo corrente*/
pcb_t* currentProc;

/***AREA MOLTO SPERIMENTALE***/
/*stato di attesa caricato se la ready queue è vuota in attesa di un nuovo processo*/
state_t waitingState;

/*la WAIT() è molto meno cpu consuming di un busy waiting*/
static void wait4proc(){
	WAIT();
}

/*Funzione per inizializzare il processo di attesa*/
#ifdef TARGET_UMPS
void initWaitingProc(){
	ownmemset(&waitingState, 0, sizeof(state_t));
	waitingState.reg_sp = (RAMTOP-(RAM_FRAMESIZE));
	/*Inizializzo sia pc_epc che reg_t9 all'entry point come dal manuale di umps*/
	waitingState.pc_epc = (memaddr)wait4proc;
	waitingState.reg_t9 = (memaddr)wait4proc;
	waitingState.status = STATUS_ALL_INT_ENABLE_KM_LT(waitingState.status);
}
#endif

#ifdef TARGET_UARM
void initWaitingProc(){
	ownmemset(&waitingState, 0, sizeof(state_t));
	waitingState.pc = (memaddr)wait4proc;
	waitingState.sp = (RAMTOP-(RAM_FRAMESIZE));
	waitingState.cpsr = (waitingState.cpsr | STATUS_SYS_MODE);
	waitingState.cpsr = STATUS_DISABLE_INT(waitingState.cpsr);
	waitingState.cpsr = STATUS_ENABLE_TIMER(waitingState.cpsr);
	waitingState.CP15_Control = CP15_DISABLE_VM(waitingState.CP15_Control);
}
#endif

/*************************/

/*In seguito sono incapsulate alcune funzioni utili di una process queue per la ready queue*/

void initReadyQueue(){
	mkEmptyProcQ(&readyQueue_h);
	initWaitingProc();
}

int emptyReadyQueue(){
	return emptyProcQ(&readyQueue_h);
}

void insertReadyQueue(pcb_t* proc){
	insertProcQ(&readyQueue_h, proc);
}

pcb_t* removeReadyQueue(){
	return removeProcQ(&readyQueue_h);
}

pcb_t* outReadyQueue(pcb_t* proc){
	return outProcQ(&readyQueue_h,proc);
}

pcb_t* headReadyQueue(){
	return headProcQ(&readyQueue_h);
}
/*******************************************************************************************/

/*Funzioni per manipolare le informazioni e gli stati relativi ai processi*/

void terminateCurrentProc(){
	if(currentProc != NULL){
		freePcb(currentProc);
		currentProc = NULL;
	}
}

void updatePriority(){
	if(currentProc != NULL){
		/*reset alla priorità del processo in controllo*/
		currentProc->priority = currentProc->original_priority;
	}
	/*update alla priorità di tutti i processi in ready queue*/
	pcb_t* p;
	list_for_each_entry(p, &readyQueue_h, p_next){
		p->priority = p->priority + AGING_AMOUNT;
	}
}
/**************************************************************************/

void schedule(){
	/*Processo in esecuzione viene messo primo in coda
	se non ci sono state modifiche alla priorità*/
	if(currentProc != NULL){
		insertReadyQueue(currentProc);
	}

	/*Se non ci sono processi da schedulare, lo scheduler attende*/
	if(emptyReadyQueue()){
		/*Carico uno stato che attende con interrupt abilitati*/
		LDST(TO_LOAD((&waitingState)));
	}
	
	/*Pop dalla ready queue diventa processo corrente e viene caricato*/
	currentProc = removeReadyQueue();
	state_t* p = &(currentProc->p_s);
	LDST(TO_LOAD(p));
}
