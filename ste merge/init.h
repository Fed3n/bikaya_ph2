#ifndef INIT_H
#define INIT_H
#include "const.h"

void initExcarea(state_t* p, void* handler);

void initProcess_KM(pcb_t* p, void* fun, int n);

void initAreas();

#endif