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
//declare kernel data structures...
int CRP; // current running PID, -1 means no process
q_t run_q, none_q; // processes ready to run and not used
pcb_t pcb[MAX_PROC]; // process table
char stack[MAX_PROC][STACK_SIZE]; // run-time stacks for processes

//(include stuff from timer lab and new PCB described in 1.html)


(some needed subroutines here, prototype them in main.h)
//InitData() still the same as PureSimulation
void InitData() {
   int i;
   /*initialize 2 queues (use MyBzero() you code in tool.c/.h)
   queue PID's 1~19 (skip 0) into none_q (un-used PID's)
   set state to NONE in all un-used pcb[1~19]
   set CRP to 0 (Idle proc ID)*/
   MyBZero(&run_q,0);
   MyBZero(&none_q,0);
   i = 1;
   for(i ; i<Q_SIZE;i++){
      pcb[i].state = NONE;
      EnQ(i,&none_q);
   }
   CRP = 0;
}

//SelectCRP() still the same as PureSimulation
void SelectCRP() { // select which PID to be new CRP
   // printf("Selcet CRP Beggineing CRP %d \n",CRP);
   /*simply return if CRP is greater than 0 (already good one selected)
   (continue only when CRP is Idle or none (0 or -1)
   */
   if(CRP > 0){
      return;
   }
   //if it's' 0 (Idle), change its state in PCB to RUN
   if(CRP == 0){
      pcb[0].state = RUN;
   }
   // printf("after checking if its zero or greater CRP %d \n",CRP);
   //if no processes to run (check size in run queue against zero)
   // printf("run_q size = %d \n",run_q.size);
   // set CRP to 0 (at least we can run Idle proc)
   if(run_q.size == 0 ){
   // printf("run_q size = %d \n ",run_q.size);
      CRP = 0;
   }else{
   // printf("after checking run q size CRP %d \n",CRP);
   //set CRP to first in run queue (dequeue it)
   CRP = DeQ(&run_q);
   // printf("after DeQ in selectcrp %d \n",CRP);
   //change mode in PCB of CRP to UMODE
   pcb[CRP].mode = UMODE;
   //change state in PCB of CRP to RUNNING
   pcb[CRP].state = RUNNING;
   }
}


//SetEntry() needed from timer lab
void SetEntry( int entry_num , func_ptr_t func_ptr){
   struct i386_gate *gateptr = &IDT_ptr_[entry_num];
   fill_gate(gateptr, (int) func_ptr, get_cs(), ACC_INTR_GATE, 0 );
}

void InitIDT(){ //is new to code, containing 3 statements from timer lab:
   //locate IDT
   fill out IDT timer entry
   program PIC mask
   (but NO "sti")
}
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

   SelectCRP();// call SelectCRP() to select process to run
   Dispatch(pcb[CRP].TF_ptr);//call Dispatch(pcb[CRP].TF_ptr) to load it and run
}

