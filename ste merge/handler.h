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

#endif
