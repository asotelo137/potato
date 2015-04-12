// isr.c, 159

#include "spede.h"
#include "type.h"
#include "isr.h"
#include "tool.h"
#include "extern.h"
#include "proc.h"
#include "syscall.h"

int wakingID;
int wake_period;
int sleepindex,sleeppid,sleepsize;
int semaID;
void CreateISR(int pid) {
  // printf("create\n");
   if(pid !=0 ){//if pid given is not 0 (Idle), enqueue it into run queue
    //   printf("Create recieved a pid : %d \n",pid);
      EnQ(pid,&run_q);
  
      // PCB of new proc:
      pcb[pid].mode = UMODE;//mode is set to UMODE
      pcb[pid].state= RUN;//state is set to RUN
      pcb[pid].runtime = 0;// both runtime counts are reset to 0
      pcb[pid].total_runtime = 0;// both runtime counts are reset to 0
      
      }
      MyBZero((char*) &mbox[pid],sizeof(mbox_t));
      MyBZero(stack[pid], STACK_SIZE); // erase stack
      // point to just above stack, then drop by sizeof(TF_t)
      pcb[pid].TF_ptr = (TF_t *)&stack[pid][STACK_SIZE];
      pcb[pid].TF_ptr--;
      // fill out trapframe of this new proc:
      if(pid == 0){
        pcb[pid].TF_ptr->eip = (unsigned int)Idle; // Idle process
      }else if(pid == 1){
        pcb[pid].TF_ptr->eip = (unsigned int)Init; // print process
      }else if(pid == 2){
        pcb[pid].TF_ptr->eip = (unsigned int)PrintDriver;
      }else{
        pcb[pid].TF_ptr->eip = (unsigned int)UserProc; // other new process
      }    
      /*if(pid == 0)
        pcb[pid].TF_ptr->eip = (unsigned int)Idle; // Idle process
      else if(pid ==1)
         pcb[pid].TF_ptr->eip = (unsigned int)PrintDriver; // print process
      else if(pid%2 == 0)
        pcb[pid].TF_ptr->eip = (unsigned int)Consumer;
      else if(pid%2 == 1)
        pcb[pid].TF_ptr->eip = (unsigned int);*/
        
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
    pcb[CRP].TF_ptr->ebx = CRP;
  return;
}

void TimerISR() {
  //dismiss timer intr (IRQ 0), otherwise, new intr signal from timer
  //won't be recognized by CPU since circuit uses edge-trigger flipflop
  //0x20 is PIC control reg, 0x60 dismisses IRQ 0
  outportb(0x20,0x60);
   
  // printf("TimerISR Beggineing CRP %d \n",CRP);
  
  //upcount the runtime of CRP and system time
  pcb[CRP].runtime++;
  sys_time++;
  sleepsize=sleep_q.size;
  while(sleepsize--){
    sleeppid=DeQ(&sleep_q);
    if(pcb[sleeppid].wake_time == sys_time){
      //int wakingID;
      EnQ(sleeppid,&run_q);
      pcb[sleeppid].state=RUN;
    }else {
      EnQ(sleeppid,&sleep_q);
    }
  }
   // just return if CRP is Idle (0) or less (-1)
   if (CRP <= 0  ){
      //printf("TIMER ISR CRP is %d\n", CRP);
      return;
   }

   if(pcb[CRP].runtime == TIME_LIMIT){
    
      pcb[CRP].total_runtime += TIME_LIMIT;
      pcb[CRP].runtime=0;
      pcb[CRP].state = RUN;
      EnQ(CRP,&run_q);
      CRP = -1;
    
   }
   // return 0;
}



void SleepISR(int seconds){
  wake_period= sys_time+(100*seconds);
  pcb[CRP].wake_time=wake_period;
  EnQ(CRP,&sleep_q);
  pcb[CRP].state=SLEEP;
  CRP=-1;
  return;
  
}
//phase 3***************************************************************
void SemWaitISR(){
  
  int semID;
  semID = pcb[CRP].TF_ptr->ebx;
  //breakpoint();
  if( semaphore[semID].count > 0){
   semaphore[semID].count --;
  }else {
    EnQ(CRP,&(semaphore[semID].wait_q));
    pcb[CRP].state = WAIT;
    CRP=-1;
  }

}
// phase 3 ***********************************************************
void SemPostISR(){
  
  int temp;
  int semID; 
  
  semID = pcb[CRP].TF_ptr->ebx;
 //breakpoint();
  if( semaphore[semID].wait_q.size ==0){
    semaphore[semID].count ++;
  }else {
    
    temp = DeQ(&semaphore[semID].wait_q);
    pcb[temp].state = RUN;
    EnQ(temp,&run_q);
  }
}

// phase 4 **********************************************************
//This function will first dismiss IRQ7 and does the same function of a semaphore-post
//(on the print_semaphore) to release the waited process which PrinterDriver() so it can resume printing

void IRQ7ISR(){
   int temp;
  int semID; 
  outportb(0x20, 0x67);
  semID = pcb[CRP].TF_ptr->ebx;
 //breakpoint();
  if( semaphore[print_semaphore].wait_q.size ==0){
    semaphore[print_semaphore].count ++;
  }else {
    
    temp = DeQ(&semaphore[print_semaphore].wait_q);
    pcb[temp].state = RUN;
    EnQ(temp,&run_q);
  }
}


// phase 4 **********************************************************
// Sem get ISR
void SemGetISR(){
  
  /*to allocate a semaphore by dequeuing the available semaphore-ID queue,
  "bzero" it, set the count to what's requested accordingly,
  and the semphore ID is returned to the calling process via its trapframe.
  */
  int gcount = pcb[CRP].TF_ptr->ebx;
  /*
  changed the if conditioncheckiing for zero to negative 1 on changs advice, remeber to include the pcb line
  inside the o condition check
  
  */
  semaID=DeQ(&semaphore_q);
  if ( semaID == -1) return;
  MyBZero((char*)&semaphore[semaID], sizeof(semaphore_t));
  semaphore[semaID].count = gcount;
  
  pcb[CRP].TF_ptr->ecx = semaID;

}

//Phase 5***********
void MsgSndISR(){
  msg_t *source, *destination;
  int msg;
  source = (msg_t *)pcb[CRP].TF_ptr->ebx;
  msg = source -> recipient;
  
  if ((mbox[msg].wait_q).size == 0){
  	
  	source->sender = CRP;
  	source->time_stamp = sys_time;
  	MsgEnQ(source, &mbox[msg].msg_q);
  }else{
  	int tmp_pid = DeQ(&(mbox[msg].wait_q));
  	EnQ(tmp_pid, &run_q);
  	
  	destination = (msg_t *)pcb[tmp_pid].TF_ptr->ebx;
  	memcpy((char*)destination,(char*)source, sizeof(msg_t));
  }
}

void MsgRcvISR(){
  
  msg_t *tmp;
  int pid;
  if(mbox[CRP].msg_q.size == 0){
    //code to block CRP
    //move the calling process to the wait queue of the mailbox, set its state, and reset cur_pid
    EnQ(CRP, &mbox[CRP].wait_q);
	pcb[CRP].state = WAIT;
	CRP=-1;
  }else{
    // dequeue a message (get a msg_t pointer) and use it to copy to CRP's local msg space!
    // copy the 1st message to the msg locally declared in the calling process
    tmp = MsgDeQ(&mbox[CRP].msg_q);
    (msg_t *)pcb[pid].TF_ptr->ebx = tmp;
  }
}
