// isr.c, 159

#include "spede.h"
#include "type.h"
#include "isr.h"
#include "tool.h"
#include "extern.h"
#include "proc.h"

void CreateISR(int pid) {
   if pid given is not 0 (Idle), enqueue it into run queue
   PCB of new proc:
      mode is set to UMODE
      state is set to RUN
      both runtime counts are reset to 0
}

void TerminateISR() {
   //just return if CRP is 0 or -1 (Idle or not given)
    if (pcb_t.state<=0){
      return 0;
    }  
   //change state of CRP to NONE
   //queue it to none queue
   //set CRP to -1 (none)
   
   pcb_t.state= NONE;
   CRP=-1;
}        

void TimerISR() {
  // just return if CRP is Idle (0) or less (-1)
   if (pcb_t.state <= 0 ){
      return 0;
   }
   
   //upcount the runtime of CRP
   pcb_t.runtime++;
   
   /*if the runtime of CRP reaches TIME_LIMIT
   (need to rotate to next PID in run queue)
      sum up runtime to the total runtime of CRP
      change its state to RUN
      queue it to run queue
      reset CRP (to -1, means none)
   */
   if(pcb_t.runtime == TIME_LIMIT){
      pcb_t.total_runtime=pcb_t.runtime + pcb_t.total_runtime;
      
   }
}
