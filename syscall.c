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

//Syscall 1, restituisce il tempo di esecuzione del processo in 3 variabili: user, kernel, prima attivazione.
void get_CPU_time(unsigned int *user, unsigned int *kernel, unsigned int *wallclock) {
	//Il kernel time viene modificato quando si entra in syscall
	currentProc->start_kernel_time = currentProc->start_kernel_time + getTODLO() - currentProc->total_kernel_time;
	
	if(user != NULL) *user = currentProc->start_user_time;
	
	if(kernel != NULL) *kernel = currentProc->total_kernel_time;

	if(wallclock != NULL) *wallclock = getTODLO - currentProc->wallclock_time;

	
}
