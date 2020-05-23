#ifndef INTERRUPT_H
#define INTERRUPT_H
#include "pcb.h"
#include "const.h"

void timerInterrupt();

void devInterrupt(int line, int dev);

void termInterrupt(int dev);

#endif
