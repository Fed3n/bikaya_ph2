#ifndef HANDLER_H
#define HANDLER_H
#include "pcb.h"
#include "syscall.h"
#include "interrupt.h"
#include "const.h"

void syscall_handler();

void interrupt_handler();

void tlb_handler();

void trap_handler();

void special_handler(int type, state_t* oldarea, unsigned int arg1, unsigned int arg2, unsigned int arg3);

#endif
