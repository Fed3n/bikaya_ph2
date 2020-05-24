#include "syscall.h"
#include "scheduler.h"
#include "auxfun.h"

extern pcb_t* currentProc;
extern int devsem[TOT_DEV_N];
extern int waitIOsem[TOT_DEV_N];

/*************************************/
/* SYSTEM CALL                       */
/*************************************/

//Ritorno i tempi passati in Kernel, User Mode e Tempo Totale (Wallclock)
void get_cpu_time(unsigned int *user, unsigned int *kernel, unsigned int *wallclock) {
	/*L'esecuzione di una Exception avviene in Kernel Mode, per cui aggiorno il Kernel Timer*/
	kernel_timer_update(currentProc);
	/*Aggiorno i valori a tempo di esecuzione*/
	if(user != NULL) *user = currentProc->total_user_timer;
	if(kernel != NULL) *kernel = currentProc->total_kernel_timer;
	if(wallclock != NULL) *wallclock = (getTODLO() - currentProc->wallclock_timer);
}

/* Crea un nuovo processo, il puntatore a pcb_t del processo appena creato
	è puntato dal campo cpid passato come parametro.
	ritorna 0 se la funzione ha successo, -1 altrimenti
*/
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

/*Termina il processo puntato da pid e tutti i suoi figli. Se il parametro è NULL
	termina il processo corrente. ritorna 0 se la funzione ha successo, -1 altrimenti 	
*/
int terminateProcess(void* pid){
	pid = (pcb_t*)pid;
	//se il pid passato è NULL devo terminare il processo corrente
	if (pid == NULL){
		pid = currentProc;
		currentProc = NULL;
	}
	if (existingProcess(pid)){
		outChildBlocked(pid); //gestisce la rimozione del processo e dei suoi figli dai semafori nei quali sono bloccati
		terminateProcess_exec(pid);
		return 0;
	} else
		return -1;
}

/* Operazione di rilascio da un semaforo */

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

/* Operazione di richiesta di un semaforo */

void passeren(int *semaddr){
	if (*semaddr <= 0){		
		if (insertBlocked(semaddr,currentProc))
			PANIC(); /*finiti i semafori*/
		currentProc = NULL;
	} else{
		currentProc->p_cskey = semaddr;
		(*semaddr)--;
	}
}

/*	Inizia un'operazione di I/O su un device.
	Il primo semaforo waitIOsem controlla che non ci sia
	un'operazione in corso, il secondo semaforo devsem
	blocca il processo che ha mandato il comando in attesa
	del completamento. Se il processo viene bloccato dal primo
	semaforo, il comando viene memorizzato nel suo pcb_t
	e verrà poi eseguito successivamente a quello in corso
	dall'interrupt handler.
*/
void do_IO(unsigned int command, unsigned int* reg, int subdevice){
	devreg_t* devp = (devreg_t*)reg;

	int i = DEVSEM_N((unsigned int)reg);

	/*se non si tratta di un terminale, quindi se i < 32*/
	if(i < (TOT_DEV_N - 2*N_DEV_PER_IL))
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

/*	Inserisce nell'area pcb_t->excarea[tipo] del currentProc gli stati
	old e new per l'handling di eccezioni speciali e marca lo used bit.
	Se used bit è già marcato il processo viene terminato.
*/
int spec_passup(int type, state_t* old, state_t* new){
	if(type < 0 || type > 2 || old == NULL || new == NULL){
		/*tipo non ammesso o state_t NULL*/
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

/*	Inserisce l'indirizzo del pcb del currentProc in pid
	e di suo padre in ppid, se pid** e ppid** esistono	
*/
void get_pid_ppid(void** pid, void** ppid){
	if(pid != NULL)
		*pid = currentProc;
	if(ppid != NULL)
		*ppid = currentProc->p_parent;
}

/*************************************/
/* FUNZIONI AUSILIARIE               */
/*************************************/

/* Termina il processo passato come parametro e tutti i suoi figli rimuovendoli dalla readyqueue se presenti. Non gestisce i processi bloccati nelle code dei semafori.
   Si occupa anche di chiamare una Verhogen prima della terminazione se il processo è in sezione critica
*/
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

/*	Ritorna 1 se il processo è un processo attualmente allocato,
	ritorna 0 altrimenti. Se il processo è in readyqueue ne viene rimosso.
*/
int existingProcess(pcb_t* p){
	/*ritorna TRUE se p è il currentProc, se è in ready queue (e lo rimuove) o se è bloccato in un semaforo*/
	return ((p == currentProc) || outReadyQueue(p) || p->p_semkey);
}
