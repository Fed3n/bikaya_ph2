#include "interrupt.h"
#include "syscall.h"
#include "handler.h"
#include "scheduler.h"

/*Struttura contenente tutti i semafori dei device, inizializzati poi a 0*/
int devsem[48];

//Aggiorno le priority, reinserisco il processo in stato ready e ripasso il controllo allo scheduler
void interrupt12(){
	/*Si aggiornano le priorità dei processi*/
	updatePriority();
	/*Si setta nuovamente il timer*/
	setTIMER(ACK_SLICE);
	/*Si richiama nuovamente lo scheduler*/
	schedule();
}

void devInterrupt(int line, int dev){
	dtpreg_t* devreg = (dtpreg_t*)DEV_REG_ADDR(line, dev);
	/*ottengo il puntatore al semaforo del device corrispondente*/
	int i = DEVSEM_N((unsigned int)devreg);
	devreg->command = CMD_ACK;
	verhogen(&devsem[i]);
}

void termInterrupt(int dev){
	termreg_t* devreg = (termreg_t*)DEV_REG_ADDR(TERMINAL_DEVICES,dev);
	int i = DEVSEM_N((unsigned int)devreg);
	unsigned int return_value = 0;
	if(devreg->recv_status == ST_RECEIVED){
		return_value = devreg->recv_status;
		devreg->recv_command = CMD_ACK;
	}
	else{
		i += N_DEV_PER_IL;
		return_value = devreg->transm_status;
		devreg->transm_command = CMD_ACK;
	}
	/*mando il valore di ritorno della Do_IO al processo*/
	pcb_t* p = headBlocked(&devsem[i]);
	p->p_s.ST_RET = return_value;

	verhogen(&devsem[i]);
	schedule();
}
