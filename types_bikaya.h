#ifndef TYPES_BIKAYA_H_INCLUDED
#define TYPES_BIKAYA_H_INCLUDED

#ifdef TARGET_UMPS
#include "umps_include/libumps.h"
#include "umps_include/arch.h"
#include "umps_include/types.h"
#endif
#ifdef TARGET_UARM
#include "uarm_include/libuarm.h"
#include "uarm_include/arch.h"
#include "uarm_include/uARMtypes.h"
#include "uarm_include/uARMconst.h"
#endif

#include "const.h"
#include "listx.h"

typedef unsigned int memaddr;

/*Struttura per memorizzare le aree degli handler speciali*/
typedef struct excarea_t{
    /*aree di memoria per newarea oldarea*/
    state_t* newarea;
    state_t* oldarea;
    int used;   /*1 quando l'area è inizializzata*/
} excarea_t;

/* Process Control Block (PCB) data structure */
typedef struct pcb_t {
    /*process queue fields */
    struct list_head p_next;

    /*process tree fields */
    struct pcb_t *   p_parent;
    struct list_head p_child, p_sib;

    /* processor state, etc */
    state_t p_s;

    /* process priority */
    int priority;
    int original_priority;

    /* key of the semaphore on which the process is eventually blocked */
    int *p_semkey;

    unsigned int start_user_timer, total_user_timer;
    unsigned int start_kernel_timer, total_kernel_timer;
    unsigned int wallclock_timer;

    /*aree per memorizzare gli handler speciali*/
    excarea_t excareas[3];
} pcb_t;



/* Semaphore Descriptor (SEMD) data structure */
typedef struct semd_t {
    struct list_head s_next;

    // Semaphore key
    int *s_key;

    // Queue of PCBs blocked on the semaphore
    struct list_head s_procQ;
} semd_t;

typedef struct semdev {
    semd_t disk[DEV_PER_INT];
    semd_t tape[DEV_PER_INT];
    semd_t network[DEV_PER_INT];
    semd_t printer[DEV_PER_INT];
    semd_t terminalR[DEV_PER_INT];
    semd_t terminalT[DEV_PER_INT];
} semdev;

#endif
