#include "const.h"
#include "pcb.h"
#include "asl.h"
#include "auxfun.h"
#include "scheduler.h"
#include "handler.h"
#include "init.h"

extern void test();
extern void print();

/*Inizializza le exception area, i PCB, mette i processi in ready queue, setta timer e poi chiama lo scheduler*/
int main(){
	initAreas();

	initPcbs();

	initASL();

	initStructs();

	pcb_t* proc_test = allocPcb();
	initProcess_KM(proc_test, test, 1);

	initReadyQueue();
	insertReadyQueue(proc_test);

	setTIMER(ACK_SLICE);

	schedule();
	
	HALT();
}
