// main.c, 159
// simulated kernel
//
// Team Name: ??????? (Members: ?????? and ??????)

#include "spede.h"      // spede stuff
#include "main.h"       // main stuff
#include "isr.h"        // ISR's
#include "tool.h"       // handy functions
#include "proc.h"       // processes such as Init()
#include "type.h"       // processes such as Init()

// kernel data structure:
int CRP;                // current running PID, -1 means no process
q_t run_q, none_q;      // processes ready to run and not used
pcb_t pcb[MAX_PROC];    // process table
char stack[MAX_PROC][STACK_SIZE]; // run-time stacks for processes

int main() {
   InitData(); 		//call Init Data to initialize kernel data
   CreateISR(0);	//call CreateISR(0) to create Idle process (PID 0)

   infinite loop: {      // alter 2 things below
      call Dispatch()    // to dispatch/run CRP
      call Kernel()      // for kernel control
   }

   return 0;
}

void InitData() {
   /*initialize 2 queues (use MyBzero() you code in tool.c/.h)
   queue PID's 1~19 (skip 0) into none_q (un-used PID's)
   set state to NONE in all un-used pcb[1~19]
   set CRP to 0 (Idle proc ID)*/
   MyBzero(run_q,Q_SIZE);
   MyBzero(none_q,Q_SIZE);
   int i = 1;
   for(i ; i<20;i++){
      pcb[i].state = NONE;
      EnQ(i,none_q)
      
   }
   CRP = 0;
}

void SelectCRP() {       // select which PID to be new CRP
   simply return if CRP is greater than 0 (already good one selected)
   (continue only when CRP is Idle or none (0 or -1)

   if it's 0 (Idle), change its state in PCB to RUN

   if no processes to run (check size in run queue against zero)
      set CRP to 0 (at least we can run Idle proc)
   else
      set CRP to first in run queue (dequeue it)

   change mode in PCB of CRP to UMODE
   change state in PCB of CRP to RUNNING
}

void Kernel() {
   int pid;
   char key;

   change state in PCB of CRP to kernel mode

   call TimerISR() to service timer interrupt as it just occurred

   check if key pressed on PC {
      read in pressed key // key = cons_getchar();
      switch(key) {
         if 'n'
            if no processes left in none queue
               "No more process!\n" (msg on target PC)
            else
               get 1st PID un-used (dequeue none queue)
               call CreateISR() with it to create new process
         if 't'
            call TerminateISR() to terminate CRP
         if 'b'
            just do breakpoint(); // this goes back to GDB prompt
         if 'q'
            just do exit(0);
     } // end switch
   } // end if some key pressed

   call SelectCRP() to settle/determine for next CRP
}

