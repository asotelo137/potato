// type.h, 159

#ifndef _TYPE_H_
#define _TYPE_H_

#define TIME_LIMIT 3         // max timer count to run
#define MAX_PROC 20          // max number of processes
#define Q_SIZE 20            // queuing capacity
#define STACK_SIZE 4096      // process stack in bytes

// this is the same as constants defined: KMODE=0, UMODE=1
typedef enum {KMODE, UMODE} mode_t;
typedef enum {NONE, RUNNING, RUN, SLEEP, WAIT, ZOMBIE} state_t;

typedef struct {             // PCB describes proc image
   mode_t mode;              // process privilege mode
   state_t state;            // state of process
   int runtime;              // run time since dispatched
   int total_runtime;        // total run time since created
} pcb_t;

typedef struct {             // proc queue type
   int head, tail, size;     // where head and tail are, and current size
   int q[Q_SIZE];            // indices into q[] array to place or get element
} q_t;

#endif
