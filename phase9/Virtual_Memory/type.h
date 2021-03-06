// type.h, 159

#ifndef _TYPE_H_
#define _TYPE_H_

#include "TF.h"

#define TIME_LIMIT 300       // max timer count to run
#define MAX_PROC 20          // max number of processes
#define Q_SIZE 20            // queuing capacity
#define STACK_SIZE 4096      // process stack in bytes

// this is the same as constants defined: KMODE=0, UMODE=1
typedef enum {KMODE, UMODE} mode_t;
typedef enum {NONE, RUNNING, RUN, SLEEP, WAIT, ZOMBIE,WAIT_CHILD} state_t;

typedef struct {              // PCB describes proc image
   mode_t mode;               // process privilege mode
   state_t state;             // state of process
   int runtime;               // run time since dispatched
   int total_runtime;         // total run time since created
   TF_t *TF_ptr;              // points to TF in stack
   int wake_time;             // amount of time to wait before wait
   int ppid;                  // parent pid
   int main_table;             // address of the main translation table of the process
} pcb_t;

typedef struct {             // proc queue type
   int head, tail, size;     // where head and tail are, and current size
   int q[Q_SIZE];            // indices into q[] array to place or get element
} q_t;

//Phase 3
/*type.h
   declare a new semaphore type "semaphore_t" that has:
      an integer used to control access, "count"
      a PID queue for waiting processes "q_t wait_q"
*/
typedef struct {
   int count;
   q_t wait_q;
} semaphore_t;

typedef struct{
   int sender;
   int recipient;
   int time_stamp;
   char data[101];
   int code;
   int number[3];
}msg_t;

typedef struct{
   msg_t msg[Q_SIZE];
   int head, tail, size;
}msg_q_t;

typedef struct{
   msg_q_t msg_q;
   q_t wait_q;
}mbox_t;

typedef struct{
   q_t TX_q,  // to transmit to terminal
       RX_q, // to receive from terminal
       echo_q; // to echo back to terminal
   int TX_sem, // transmit space available count
       RX_sem, // receive data (arrived) count
       echo, // to echo back to terminal (the typing) or not
       TX_extra; // if 1, TXRDY event occurred but echo_q and TX_q were empty
}terminal_t;


typedef void (* func_ptr_t)(); // void-returning function pointer type
//phase 8 page type//////////////////////////////////////////////////////
typedef struct{
   int owner,
       addr;
}page_t;

#endif _TYPE_H_
