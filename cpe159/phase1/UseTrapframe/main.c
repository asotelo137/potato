// CpE/CSc159
// UseTrapframe/main.txt
//
// skeleton of main() for phase 1

//include statements...
#include "spede.h" // spede stuff
#include "main.h" // main stuff
#include "isr.h" // ISR's
#include "tool.h" // handy functions
#include "proc.h" // processes such as Init()
#include "type.h" // processes such as Init()
// kernel data structure:
int CRP; // current running PID, -1 means no process
q_t run_q, none_q; // processes ready to run and not used
pcb_t pcb[MAX_PROC]; // process table
char stack[MAX_PROC][STACK_SIZE]; // run-time stacks for processes

//declare kernel data structures...
//(include stuff from timer lab and new PCB described in 1.html)

(some needed subroutines here, prototype them in main.h)
InitData() still the same as PureSimulation

SelectCRP() still the same as PureSimulation

SetEntry() needed from timer lab

InitIDT() is new to code, containing 3 statements from timer lab:
   locate IDT
   fill out IDT timer entry
   program PIC mask
   (but NO "sti")

int main() {
   call InitData() to initialize kernel data structure
   call (new) InitIDT() to set up timer (from timer lab)
   call CreateISR() to create Idle proc
   call Dispatch(pcb[...) to load Idle proc to run

   return 0;
} // main ends

void Kernel(TF_t *TF_ptr) {
   change to kernel mode for CRP
   save TF_ptr to PCB of CRP

   switch upon intr_num pointed by TF_ptr to call corresponging ISR {
      if it's TIMER_INTR:
         call TimerISR()
      default:
         PANIC! msg and break into GDB
   }

// still handles other keyed-in simulated events
   (same as PureSimulation to handle key events)

   call SelectCRP() to select process to run
   call Dispatch(pcb[CRP].TF_ptr) to load it and run
}

