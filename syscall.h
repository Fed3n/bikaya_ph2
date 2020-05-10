#ifndef SYSCALL_H
#define SYSCALL_H
#include "pcb.h"
#include "const.h"

void sys3();

void sys3_exec(pcb_t* root);

void get_CPU_time(unsigned int *user, unsigned int *kernel, unsigned int *wallclock);

#endif
