#ifndef SYSCALL_H
#define SYSCALL_H
#include "pcb.h"
#include "asl.h"
#include "const.h"

void get_cpu_time(unsigned int *user, unsigned int *kernel, unsigned int *wallclock);

int createProcess(state_t* statep, int priority, void **cpid);

int terminateProcess(void* pid);

void verhogen(int *semaddr);

void passeren(int *semaddr);

void do_IO(unsigned int command, unsigned int* reg, int subdevice);

int spec_passup(int type, state_t* old, state_t* new);

void get_pid_ppid(void** pid, void** ppid);

void terminateProcess_exec(pcb_t *root);

int existingProcess(pcb_t* p);

#endif
