#include "interrupt.h"
#include "handler.h"
#include "scheduler.h"

//Aggiorno le priority, reinserisco il processo in stato ready e ripasso il controllo allo scheduler
void interrupt12(){
	/*Si aggiornano le priorità dei processi*/
	updatePriority();
	/*Si setta nuovamente il timer*/
	setTIMER(ACK_SLICE);
	/*Si richiama nuovamente lo scheduler*/
	schedule();
}
