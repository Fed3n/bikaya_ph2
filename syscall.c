#include "syscall.h"
#include "scheduler.h"

extern pcb_t* currentProc;

//si occupa di terminare il processo corrente e di rimuovere tutti i figli dalla ready queue
void sys3(){
	if (currentProc != NULL) {
		sys3_exec(currentProc);
		currentProc = NULL;
	}
	schedule();
}

//funzione ausiliaria utilizzata per poter realizzare la ricorsione
void sys3_exec(pcb_t* root){
	while (!emptyChild(root)){
		pcb_t* child = removeChild(root);
		if (child != NULL) 
			sys3_exec(child);
	}		
	outReadyQueue(root);
	freePcb(root);
}
