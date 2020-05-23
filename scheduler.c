#include "pcb.h"
#include "scheduler.h"
#include "syscall.h"
#include "auxfun.h"

/*la testa della ready queue è hidden e vi ci si interfaccia tramite le funzioni dello scheduler*/
HIDDEN LIST_HEAD(readyQueue_h);

/*Puntatore a processo corrente, potrebbe essere NULL*/
pcb_t* currentProc;

/*In seguito sono incapsulate alcune funzioni utili di una process queue per la ready queue*/

void initReadyQueue(){
	mkEmptyProcQ(&readyQueue_h);
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

	/*Se non ci sono processi da schedulare, lo scheduler attende e abilita interrupt*/
	if(emptyReadyQueue()){
		setSTATUS(STATUS_ENABLE_ALL_INT(getSTATUS()));
		WAIT();
	}	

	//Avvio il processo in user mode
	start_user_mode(currentProc);

	/*Pop dalla ready queue diventa processo corrente e viene caricato*/
	currentProc = removeReadyQueue();

	setTIMER(ACK_SLICE);
	state_t* p = &(currentProc->p_s);
	LDST(TO_LOAD(p));
}
