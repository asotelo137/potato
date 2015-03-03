// extern.h, 159

#ifndef _EXTERN_H_
#define _EXTERN_H_

#include "type.h"            // q_t, pcb_t, MAX_PROC, STACK_SIZE

extern int CRP,sys_time,product,product_semaphore;              // PID of currently-running process, -1 means none
extern q_t run_q, none_q,sleep_q,semaphore_q;    // ready to run, not used proc IDs
extern pcb_t pcb[MAX_PROC];  // process table
extern char stack[MAX_PROC][STACK_SIZE]; // proc run-time stacks
extern semaphore_t semaphore[MAX_PROC];
extern semaphoreID;

#endif
