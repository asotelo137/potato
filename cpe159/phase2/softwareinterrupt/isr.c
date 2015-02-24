// isr.c, 159

#include "spede.h"
#include "type.h"
#include "isr.h"
#include "tool.h"
#include "extern.h"
#include "proc.h"
#include "syscall.c"

int wakingID;
int wake_period;
void CreateISR(int pid) {
  // printf("create\n");
   if(pid !=0 ){//if pid given is not 0 (Idle), enqueue it into run queue
    //   printf("Create recieved a pid : %d \n",pid);
      EnQ(pid,&run_q);
   }
      // PCB of new proc:
      pcb[pid].mode = UMODE;//mode is set to UMODE
      pcb[pid].state= RUN;//state is set to RUN
      pcb[pid].runtime = 0;// both runtime counts are reset to 0
      pcb[pid].total_runtime = 0;// both runtime counts are reset to 0
      
      
      MyBzero(stack[pid], STACK_SIZE); // erase stack
      // point to just above stack, then drop by sizeof(TF_t)
      pcb[pid].TF_ptr = (TF_t *)&stack[pid][STACK_SIZE];
      pcb[pid].TF_ptr--;
      // fill out trapframe of this new proc:
      if(pid == 0)
      pcb[pid].TF_ptr->eip = (unsigned int)Idle; // Idle process
      else
      pcb[pid].TF_ptr->eip = (unsigned int)UserProc; // other new process
      pcb[pid].TF_ptr->eflags = EF_DEFAULT_VALUE | EF_INTR;
      pcb[pid].TF_ptr->cs = get_cs();
      pcb[pid].TF_ptr->ds = get_ds();
      pcb[pid].TF_ptr->es = get_es();
      pcb[pid].TF_ptr->fs = get_fs();
      pcb[pid].TF_ptr->gs = get_gs();
      
   
//  return 0;
}

void TerminateISR() {
  //printf("terminate\n");
   //just return if CRP is 0 or -1 (Idle or not given)
    
    if (CRP <=0 ){
      return;
    }  
   //change state of CRP to NONE
   pcb[CRP].state = NONE;
   //queue it to none queue
   EnQ(CRP,&none_q);                             //STILL ERROR HERE
   //set CRP to -1 (none)
   CRP = -1;
 //  return 0;
}    

void GetPidISR(){
  
  outportb(0x20,0x60);
  pcb[CRP].TF_ptr->ebx = CRP;
  GetPid();
  
  return;
}

void TimerISR() {
   outportb(0x20,0x60);
   
  // printf("TimerISR Beggineing CRP %d \n",CRP);
  
   //upcount the runtime of CRP and system time
   pcb[CRP].runtime++;
   sys_time++;
   while(sleep_q.size !=0 && pcb[sleep_q.q[sleep_q.head]].wake_time <= sys_time){
     //int wakingID;
     wakingID = DeQ(&sleep_q);
     pcb[wakingID].state=RUN;
     EnQ(wakingID,&run_q);
   }
   // just return if CRP is Idle (0) or less (-1)
   if (CRP <= 0  ){
      //printf("TIMER ISR CRP is %d\n", CRP);
      return;
   }

   if(pcb[CRP].runtime == TIME_LIMIT){
    
      pcb[CRP].total_runtime=pcb[CRP].runtime + pcb[CRP].total_runtime;
      pcb[CRP].runtime=0;
      pcb[CRP].state = RUN;
      EnQ(CRP,&run_q);
      CRP = -1;
    
   }
   // return 0;
}



void SleepISR(int seconds){
  outportb(0x20,0x60);
  wake_period= sys_time+(100*seconds);
  pcb[CRP].wake_time=wake_period;
  EnQ(CRP,&sleep_q);
  pcb[CRP].state=SLEEP;
  CRP=-1;
  return;
  
}
