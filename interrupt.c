#include "interrupt.h"
#include "syscall.h"
#include "handler.h"
#include "scheduler.h"

/*Struttura contenente tutti i semafori dei device, inizializzati poi a 0*/
int devsem[TOT_DEV_N];
int waitIOsem[TOT_DEV_N];
extern pcb_t* currentProc;

//Aggiorno le priority, reinserisco il processo in stato ready e ripasso il controllo allo scheduler
void timerInterrupt(){
	/*Si aggiornano le priorità dei processi*/
	updatePriority();
}

void devInterrupt(int line, int dev){
	dtpreg_t* devreg = (dtpreg_t*)DEV_REG_ADDR(line, dev);
	/*ottengo il puntatore al semaforo del device corrispondente*/
	int i = DEVSEM_N((unsigned int)devreg);
	unsigned int return_value;
	return_value = devreg->status;
	devreg->command = CMD_ACK;

	/*mando il valore di ritorno della Do_IO al processo*/
	pcb_t* p = headBlocked(&devsem[i]);
	p->p_s.ST_RET = return_value;

	verhogen(&devsem[i]);

	/****questa parte purtroppo non viene testata nel test****/
	/*se c'erano operazioni di IO in sospeso gestisco la prima sul semaforo*/
	pcb_t* q;
	if((p = headBlocked(&waitIOsem[i]))){
		/*recupero il comando in sospeso*/
		unsigned int command = p->suspendedCmd;
		devreg->command = command;
		/*rilascio il semaforo di attesa per l'operazione*/
		verhogen(&waitIOsem[i]);
		/*operazione un po' poco chiara, ma la passeren mette in coda un currentProc
		mentre qua il processo che metto in coda non è un currentProc, quindi
		cambio momentaneamente il currentProc*/
		q = currentProc;
		currentProc = p;
		/*e lo metto in attesa del completamento del comando*/
		passeren(&devsem[i]);
		currentProc = q;
	}
	else
		/*altrimenti V su un semaforo senza processi in attesa*/
		verhogen(&waitIOsem[i]);
	/*********************************************************/
}

void termInterrupt(int dev){
	termreg_t* devreg = (termreg_t*)DEV_REG_ADDR(TERMINAL_DEVICES,dev);
	int i = DEVSEM_N((unsigned int)devreg);
	int isRecv = 1;
	unsigned int return_value;
	if(devreg->recv_status == ST_RECEIVED){
		return_value = devreg->recv_status;
		devreg->recv_command = CMD_ACK;
	}
	else{
		i += N_DEV_PER_IL;
		return_value = devreg->transm_status;
		devreg->transm_command = CMD_ACK;
		isRecv--;
	}
	/*mando il valore di ritorno della Do_IO al processo*/
	pcb_t* p = headBlocked(&devsem[i]);
	p->p_s.ST_RET = return_value;

	verhogen(&devsem[i]);

	/****questa parte purtroppo non viene testata nel test****/
	/*se c'erano operazioni di IO in sospeso gestisco la prima sul semaforo*/
	pcb_t* q;
	if((p = headBlocked(&waitIOsem[i]))){
		/*recupero il comando in sospeso*/
		unsigned int command = p->suspendedCmd;
		if(isRecv)
			devreg->recv_command = command;
		else
			devreg->transm_command = command;
		/*rilascio il semaforo di attesa per l'operazione*/
		verhogen(&waitIOsem[i]);
		/*operazione un po' poco chiara, ma la passeren mette in coda un currentProc
		mentre qua il processo che metto in coda non è un currentProc, quindi
		cambio momentaneamente il currentProc*/
		q = currentProc;
		currentProc = p;
		/*e lo metto in attesa del completamento del comando*/
		passeren(&devsem[i]);
		currentProc = q;
	}
	else
		/*altrimenti V su un semaforo senza processi in attesa*/
		verhogen(&waitIOsem[i]);
	/*********************************************************/
}
