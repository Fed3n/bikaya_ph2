#ifndef SYSCALL_H
#define SYSCALL_H
#include "pcb.h"
#include "const.h"

typedef struct excarea_t{
	/*aree di memoria per newarea oldarea*/
	state_t* newarea;
	state_t* oldarea;
	int used;	/*1 quando l'area è inizializzata*/
} excarea_t;

int testval(int n);

void sys3();

int sys7(int type, state_t* old, state_t* new);

void initSysData();

void sys3_exec(pcb_t* root);

#endif
