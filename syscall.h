#ifndef SYSCALL_H
#define SYSCALL_H
#include "pcb.h"
#include "asl.h"
#include "const.h"

void kernel_timer_update(pcb_t *currentProc);

void user_timer_update(pcb_t *currentProc);

void get_cpu_time(unsigned int *user, unsigned int *kernel, unsigned int *wallclock);

void createProcess(state_t* statep, int priority, void **cpid);

void terminateProcess();

void terminateProcess_exec(pcb_t *root);

void verhogen(int *semaddr);

void passeren(int *semaddr);

#endif
