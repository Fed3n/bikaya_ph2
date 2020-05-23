#include "auxfun.h"

void ownmemset(void *mem, char val, unsigned int size){
	int i;
	for(i = 0; i < size; i++){
		/*Assegno ad ogni byte di mem il valore val*/
		((char*)mem)[i] = val;
	}
}

void ownmemcpy(void *src, void *dest, unsigned int size){
	int i;
	for(i = 0; i < size; i++){
		((char*)dest)[i] = ((char*)src)[i];
	}
}

/*Time management*/

/*Aggiorna lo User Time ed entra in Kernel Mode*/
void user_timer_update(pcb_t *currentProc) {
	if(currentProc != NULL) { 
		/*Calcola iol tempo passato in User Mode*/
		if(currentProc->start_user_timer > 0) {
			currentProc->total_user_timer = currentProc->total_user_timer + (getTODLO() - currentProc->start_user_timer);
			//Inizializza a 0 il Timer corrente del processo User e si esce dalla User Mode
			currentProc->start_user_timer = 0;
		}
	//Si entra in Kernel Mode
	currentProc->start_kernel_timer = getTODLO();
	}
}

/*Aggiorna il Kernel Timer ed entra in User Mode*/
void kernel_timer_update(pcb_t *currentProc) {
	if(currentProc != NULL) { 
		/*Calcola il tempo passato in Kernel Mode*/
		if(currentProc->start_kernel_timer > 0) {
			currentProc->total_kernel_timer = currentProc->total_kernel_timer + (getTODLO() - currentProc->start_kernel_timer);
			/*Inizializza a 0 il Timer corrente del processo Kernel e si esce dalla Kernel Mode*/
			currentProc->start_user_timer = 0;
		}
	/*Si entra in user mode*/
	currentProc->start_user_timer = getTODLO();
	}
}
