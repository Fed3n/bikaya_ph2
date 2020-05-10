#ifndef SYSCALL_H
#define SYSCALL_H
#include "pcb.h"
#include "asl.h"
#include "const.h"

typedef struct excarea_t{
	/*aree di memoria per newarea oldarea*/
	state_t* newarea;
	state_t* oldarea;
	int used;	/*1 quando l'area è inizializzata*/
} excarea_t;

void kernel_timer_update(pcb_t *currentProc);

void get_cpu_time(unsigned int *user, unsigned int *kernel, unsigned int *wallclock);

void createProcess(state_t* statep, int priority, void **cpid);

void terminateProcess();

void terminateProcess_exec(pcb_t *root);

void verhogen(int *semaddr);

void passeren(int *semaddr);

int sys7(int type, state_t* old, state_t* new);

void initSysData();
#endif
