#ifndef SYSCALL_H
#define SYSCALL_H
#include "pcb.h"
#include "const.h"

void createProcess(state_t* statep, int priority, void **cpid);

void terminateProcess();

void terminateProcess_exec(pcb_t *root);

void verhogen(int *semaddr);

void passeren(int *semaddr);

#endif
