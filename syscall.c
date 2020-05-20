#include "syscall.h"
#include "scheduler.h"
#include "auxfun.h"

extern pcb_t* currentProc;
extern int devsem[TOT_DEV_N];
extern int waitIOsem[TOT_DEV_N];

/*************************************/
/* SYSTEM CALL                       */
/*************************************/

//Ritorno i tempi passati in kernel, user mode e tempo totale (wallclock)
void get_cpu_time(unsigned int *user, unsigned int *kernel, unsigned int *wallclock) {
	//Entro in user mode ed aggiorno il tempo passato in kernel mode
	user_timer_update(currentProc);
	if(user != NULL) *user = currentProc->total_user_timer;
	if(kernel != NULL) *kernel = currentProc->total_kernel_timer;
	if(wallclock != NULL) *wallclock = (getTODLO() - currentProc->wallclock_timer);
}

//crea un nuovo processo
int createProcess(state_t* statep, int priority, void** cpid){
	pcb_t* proc = allocPcb();
	if (proc == NULL)
		return -1;
	ownmemcpy(statep, &(proc->p_s), sizeof(state_t));
	proc->original_priority = priority;
	proc->priority = priority;
	proc->wallclock_timer = getTODLO();
	insertChild(currentProc,proc);
	insertReadyQueue(proc);
	if (cpid != NULL)
		*cpid = proc;
	return 0;
}

//si occupa di terminare il processo corrente e di rimuovere tutti i figli dalla ready queue
int terminateProcess(void* pid){
	pid = (pcb_t*)pid;
	if (pid == NULL){
		pid = currentProc;
		currentProc = NULL;
	}
	if (existingProcess(pid)){
		outChildBlocked(pid); //gestisce la rimozione del processo e dei figli dai semafori su cui sono bloccati
		terminateProcess_exec(pid);
		return 0;
	} else
		return -1;
}

//rilascio del semaforo
void verhogen(int *semaddr){
	if (headBlocked(semaddr) != NULL){
		pcb_t *p = removeBlocked(semaddr);
		//indico con il campo p_cskey che sono entrato in sezione critica del semaforo su cui ero bloccato
		p->p_cskey = semaddr;
		//aggiorno il campo p_cskey per indicare che il processo corrente (che ha chiamato la Verhogen) 
		//è uscito dalla sezione critica del semaforo in questione
		if (currentProc != NULL){
			currentProc->p_cskey = NULL;
			currentProc->p_semkey = NULL;
		}
		insertReadyQueue(p);
	} else{
		(*semaddr)++;
	}
}	

//richiesta di un semaforo
void passeren(int *semaddr){
	if (*semaddr <= 0){		
		if (insertBlocked(semaddr,currentProc))
			PANIC(); /*finiti i semafori*/
		currentProc = NULL;
	} else{
		(*semaddr)--;
	}
}

void do_IO(unsigned int command, unsigned int* reg, int subdevice){
	devreg_t* devp = (devreg_t*)reg;

	int i = DEVSEM_N((unsigned int)reg);

	/*se non si tratta di un terminale*/
	if(i < (TOT_DEV_N- 2*N_DEV_PER_IL))
		subdevice = 1;
	/****questa parte purtroppo non viene testata nel test****/
	/*Controllo che il device non sia in uso*/
	/*Se era in uso l'invio del comando verrà gestito dall'interrupt handler
	al termine dell'operazione*/
	currentProc->suspendedCmd = command;
	/*se deve essere sospeso non va oltre a questa P*/
	passeren(&waitIOsem[i]);
	/*********************************************************/

	/*controllo quale subdevice è nel caso di un terminale*/
	if(subdevice)
		/*se è un device generico devp è una union, manderò il comando
		al campo corretto in ogni caso*/
		devp->term.recv_command = command;
	else{
		/*se devo trasmettere allora scalo di 8 semafori per quello giusto*/
		i += N_DEV_PER_IL;
		devp->term.transm_command = command;
	}
	passeren(&devsem[i]);
}

int spec_passup(int type, state_t* old, state_t* new){
	if(type < 0 || type > 2){
		/*tipo non ammesso*/
		return -1;
	}
	excarea_t* p = &(currentProc->excareas[type]);
	/*se used è marcato termino chiamante*/
	if(p->used == 1){
		terminateProcess(NULL);
	}
	/*altrimenti inizializzo le aree del type corrispondente e marco used*/
	else{
		p->used = 1;
		p->oldarea = old;
		p->newarea = new;
	}
	return 0;
}

void get_pid_ppid(void** pid, void** ppid){
	if(pid != NULL)
		*pid = currentProc;
	if(ppid != NULL)
		*ppid = currentProc->p_parent;
}

/*************************************/
/* FUNZIONI AUSILIARIE               */
/*************************************/

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


//funzione ausiliaria utilizzata per poter realizzare la ricorsione
void terminateProcess_exec(pcb_t *root){
	while (!emptyChild(root)){
		pcb_t *child = removeChild(root);
		if (child != NULL) 
			terminateProcess_exec(child);
	}		
	if (root->p_cskey != NULL)  //chiamo una V se il processo da eliminare si trova in sezione critica
			SYSCALL(VERHOGEN, (int)root->p_cskey, 0, 0);
	outReadyQueue(root);
	freePcb(root);
}

//ritorna 1 se il processo è un processo attualmente allocato 
//ritorna 0 altrimenti
int existingProcess(pcb_t* p){
	int exists = 0;
	if (p == currentProc)
		exists = 1;
	pcb_t *proc = NULL;
	proc = outReadyQueue(p); //controlla se il processo è presente in readyQueue
	if (proc != NULL)
		exists = 1;
	if (p->p_semkey != NULL) //controllo se il processo è bloccato su di un semaforo
		exists = 1;
	return exists;
}
