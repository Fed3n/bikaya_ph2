#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "const.h"

#define AGING_AMOUNT 1

void initReadyQueue();

int emptyReadyQueue();

void insertReadyQueue(pcb_t* proc);

pcb_t* removeReadyQueue();

pcb_t* outReadyQueue(pcb_t* proc);

pcb_t* headReadyQueue();

void terminateCurrentProc();

void terminateProc();

void updatePriority();

void schedule();
#endif
