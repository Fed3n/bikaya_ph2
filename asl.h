#ifndef ASL_H
#define ASL_H

#include "types_bikaya.h"
#include "const.h"
#include "listx.h"

/* ASL handling functions */
semd_t* getSemd(int *key);
void initASL();

int insertBlocked(int *key, pcb_t* p);
pcb_t* removeBlocked(int *key);
pcb_t* outBlocked(pcb_t *p,int flag);
pcb_t* headBlocked(int *key);
void outChildBlocked(pcb_t *p);

#endif
