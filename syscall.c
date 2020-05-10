#include "syscall.h"
#include "scheduler.h"
#include "auxfun.h"

//Time management

void kernel_timer_update(pcb_t *currentProc) {
	if(currentProc != NULL) { 
		//Calcolo il tempo passato in user prima di passare alla kernel mode
		if(currentProc->start_user_timer > 0) {
			currentProc->total_user_timer = currentProc->total_user_timer + (getTODLO() - currentProc->start_user_timer);
			//Esco dalla user mode	
			currentProc->start_user_timer = 0;
		}
	//Entro in kernel mode
	currentProc->start_kernel_timer = getTODLO();
	}
}

void user_timer_update(pcb_t *currentProc) {
	if(currentProc != NULL) { 
		//Calcolo il tempo passato in kernel prima di passare alla user mode 
		if(currentProc->start_kernel_timer > 0) {
			currentProc->total_kernel_timer = currentProc->total_kernel_timer + (getTODLO() - currentProc->start_kernel_timer);
			//Esco dalla kernel mode	
			currentProc->start_user_timer = 0;
		}
	//Entro in user mode
	currentProc->start_user_timer = getTODLO();
	}
}

//definizione per i test

#ifdef TARGET_UMPS
extern void termprint(char *str);
#endif

#ifdef TARGET_UARM
#define termprint(str) tprint(str);
#endif

#ifdef TARGET_UMPS
#define p_s.pc p_s.pc_epc
#endif

extern pcb_t* currentProc;
excarea_t excareas[3];

//Ritorno i tempi passati in kernel, user mode e tempo totale (wallclock)
void get_cpu_time(unsigned int *user, unsigned int *kernel, unsigned int *wallclock) {
	//Entro in user mode ed aggiorno il tempo passato in kernel mode
	user_timer_update(currentProc);
	if(user != NULL) *user = currentProc->total_user_timer;
	if(kernel != NULL) *kernel = currentProc->total_kernel_timer;
	if(wallclock != NULL) *wallclock = currentProc->wallclock_timer;
}

//crea un nuovo processo
void/*int*/ createProcess(state_t* statep, int priority, void** cpid){
	pcb_t* proc = allocPcb();
	/*if (proc == NULL)
		return -1;*/
	ownmemcpy(statep, &(proc->p_s), sizeof(state_t));
	proc->original_priority = priority;
	proc->priority = priority;

	proc->wallclock_timer = getTODLO();
	
	insertChild(currentProc,proc);
	insertReadyQueue(proc);
	if (cpid != NULL)
		*cpid = &proc;
	//rerurn 0;
	schedule();
}

//si occupa di terminare il processo corrente e di rimuovere tutti i figli dalla ready queue
void terminateProcess(void* pid){
	//if (pid    bisogna fare una funzione che mi dica se un pcb_t è allocato o no
	if (pid == NULL) {
		terminateProcess_exec(currentProc);
		currentProc = NULL;
	} else
		terminateProcess_exec(pid);
	//return 0;
	schedule();
}

//funzione ausiliaria utilizzata per poter realizzare la ricorsione
void terminateProcess_exec(pcb_t *root){
	while (!emptyChild(root)){
		pcb_t *child = removeChild(root);
		if (child != NULL) 
			terminateProcess_exec(child);
	}		
	outReadyQueue(root);
	freePcb(root);
}

//rilascio del semaforo
void verhogen(int *semaddr){
	if (headBlocked(semaddr) != NULL){
		pcb_t *p = removeBlocked(semaddr);
		if (p != NULL)
			insertReadyQueue(p);
	} else {
		
		termprint("semaforo  vuoto");

		(*semaddr)++;
	}
	schedule();
}	

//richiesta di un semaforo
void passeren(int *semaddr){
	if (*semaddr <= 0){		
		if (insertBlocked(semaddr,currentProc))
			termprint("ERROR: no more semaphores available!");
	}else
		(*semaddr)--;
	schedule();
}

int sys7(int type, state_t* old, state_t* new){
	termprint("sys7 called\n");
	/*magari controllo che 0 <= type <= 2*/
	excarea_t* p = &(excareas[type]);
	/*se used è marcato termino chiamante*/
	if(p->used == 1)
		terminateProcess(NULL);
	/*altrimenti inizializzo le aree del type corrispondente e marco used*/
	else{
		p->used = 1;
		p->newarea = new;
		p->oldarea = old;
	}
	termprint("Calling schedule after sys7...\n");
	schedule();
}

void initSysData(){
	int i;
	for(i = 0; i < 3; i++){
		ownmemset(&excareas[i], 0, sizeof(excarea_t));
	}
}

