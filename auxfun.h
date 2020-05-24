#include "types_bikaya.h"

#ifndef AUXFUN_H
#define AUXFUN_H

/*Funzioni ausiliarie per il Bikaya Project*/

/*Semplice implementazione di memset() per evitare l'uso di librerie*/
void ownmemset(void *mem, char val, unsigned int size);
void ownmemcpy(void *src, void *dest, unsigned int size);
void kernel_timer_update(pcb_t *currentProc);
void user_timer_update(pcb_t *currentProc);

#endif
