#include "syscall.h"
#include "scheduler.h"
#include "auxfun.h"

#ifdef TARGET_UMPS
extern void termprint(char *str);
#endif

#ifdef TARGET_UARM
#define termprint(str) tprint(str);
#endif

extern pcb_t* currentProc;

excarea_t excareas[3];

int testval(int n){
	return excareas[n].used;
}

//si occupa di terminare il processo corrente e di rimuovere tutti i figli dalla ready queue
void sys3(){
	termprint("sys3 called\n");
	if (currentProc != NULL) {
		sys3_exec(currentProc);
		currentProc = NULL;
	}
	schedule();
}


int sys7(int type, state_t* old, state_t* new){
	termprint("sys7 called\n");
	/*magari controllo che 0 <= type <= 2*/
	excarea_t* p = &(excareas[type]);
	/*se used è marcato termino chiamante*/
	if(p->used == 1)
		sys3();
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