#ifndef SYSCALL_H
#define SYSCALL_H
#include "pcb.h"
#include "const.h"

void sys3();

void sys3_exec(pcb_t* root);

#endif
