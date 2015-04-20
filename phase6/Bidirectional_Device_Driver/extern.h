// extern.h, 159

#ifndef _EXTERN_H_
#define _EXTERN_H_

#include "type.h"            // q_t, pcb_t, MAX_PROC, STACK_SIZE

extern int CRP,sys_time;              // PID of currently-running process, -1 means none
extern q_t run_q, none_q,sleep_q;    // ready to run, not used proc IDs
extern pcb_t pcb[MAX_PROC];  // process table
extern char stack[MAX_PROC][STACK_SIZE]; // proc run-time stacks

//phase 3
/*main.c/extern.h
   apply the semaphore type to declare an array of semaphores,
   "semaphore," which has the same size as a PID queue

   a semaphore queue "semaphore_q" to be primed with available
   semaphores (numbers 1 ~ 19),
    
   need for testing (Phase 3 only, after Phase, delete):
      integers "product_semaphore" and "product"
*/
extern q_t semaphore_q;

extern semaphore_t sem[Q_SIZE];

extern semaphore_t semaphore[Q_SIZE];

extern int product_semaphore,product,semaphoreID;
//phase 4 printing globals
extern int print_it,print_semaphore;

//phase 5 ****************************************88
extern mbox_t mbox[MAX_PROC];

#endif
