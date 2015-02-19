// isr.c, 159

#include "spede.h"
#include "type.h"
#include "isr.h"
#include "tool.h"
#include "extern.h"
#include "proc.h"

void CreateISR(int pid) {
   printf("create\n");
   if(pid !=0 ){ //if pid given is not 0 (Idle), enqueue it into run queue
      EnQ(pid,&run_q);
      // PCB of new proc:
      pcb[pid].mode = UMODE;//mode is set to UMODE
      pcb[pid].state= RUN;//state is set to RUN
      pcb[pid].runtime = 0;// both runtime counts are reset to 0
      pcb[pid].total_runtime = 0;// both runtime counts are reset to 0
   }
//  return 0;
}

void TerminateISR() {
   printf("terminate\n");
   //just return if CRP is 0 or -1 (Idle or not given)
    if (CRP == 0 || CRP < -1 ){
      return;
    }  
   //change state of CRP to NONE
   CRP = NONE;
   //queue it to none queue
   EnQ(CRP,&none_q);                             //STILL ERROR HERE
   //set CRP to -1 (none)
   CRP = -1;
 //  return 0;
}        

void TimerISR() {
   printf("timer\n");
  // just return if CRP is Idle (0) or less (-1)
   if (CRP <= 0  ){
      printf("TIMER ISR CRP is %d\n", CRP);
      return;
   }
   
   //upcount the runtime of CRP
   pcb[CRP].runtime++;
   printf("runtime %d \n", pcb[CRP].runtime);
   
   /*if the runtime of CRP reaches TIME_LIMIT
   (need to rotate to next PID in run queue)
      sum up runtime to the total runtime of CRP
      change its state to RUN
      queue it to run queue
      reset CRP (to -1, means none)
   */
   if(pcb[CRP].runtime == TIME_LIMIT){
      printf("limit\n");
      pcb[CRP].total_runtime=pcb[CRP].runtime + pcb[CRP].total_runtime;
      pcb[CRP].state = RUN;
      EnQ(CRP,&run_q);
      CRP = -1;
      printf("set crp -1\n");
   }
   // return 0;
}
