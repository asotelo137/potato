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
#include "entry.h"


#include <spede/stdio.h>
#include <spede/flames.h>
#include <spede/machine/io.h>
#include <spede/machine/proc_reg.h>
#include <spede/machine/seg.h>
#include <spede/machine/pic.h>

// kernel data structure:
int CRP;                // current running PID, -1 means no process
q_t run_q, none_q;      // processes ready to run and not used
pcb_t pcb[MAX_PROC];    // process table
char stack[MAX_PROC][STACK_SIZE]; // run-time stacks for processes
//(include stuff from timer lab and new PCB described in 1.html)



struct i386_gate *IDT_ptr;
void InitIDT();
//SetEntry() needed from timer lab
void SetEntry(int entry_num, func_ptr_t func_ptr){
   struct i386_gate *gateptr = &IDT_ptr[entry_num];
   fill_gate(gateptr, (int)func_ptr, get_cs(), ACC_INTR_GATE,0);
}
//InitData() still the same as PureSimulation
void InitData() {
   int i;
   MyBzero(&run_q,0);
   MyBzero(&none_q,0);
   
   for(i =1  ; i<Q_SIZE;i++){
      pcb[i].state = NONE;
      EnQ(i,&none_q);
   }
   CRP = 0;
}

//SelectCRP() still the same as PureSimulation
void SelectCRP() {       // select which PID to be new CRP
   if(CRP > 0){
      return;
   }    
   //if it's' 0 (Idle), change its state in PCB to RUN
   if(CRP == 0){
      pcb[0].state = RUN;
   }
   
   if(run_q.size == 0 ){
   //      printf("run_q size = %d \n ",run_q.size);
      CRP = 0;
   }else{  
      //       printf("after checking run q size CRP %d \n",CRP);
      //set CRP to first in run queue (dequeue it)
      CRP = DeQ(&run_q); 
      //      printf("after DeQ in selectcrp %d \n",CRP);
      //change mode in PCB of CRP to UMODE
      pcb[CRP].mode = UMODE;
      //change state in PCB of CRP to RUNNING
      pcb[CRP].state = RUNNING;
   }
}


void InitIDT(){ //is new to code, containing 3 statements from timer lab:
   IDT_ptr = get_idt_base(); //locate IDT
   SetEntry(32, TimerEntry); //fill out IDT timer entry
   outportb(0x21, ~0x01); //program PIC mask
   //(but NO "sti")
}

int main() {
   InitData(); 		//call Init Data to initialize kernel data
   
   CreateISR(0);	//call CreateISR(0) to create Idle process (PID 0)
   
   InitIDT();        //call (new) InitIDT() to set up timer (from timer lab)
   
       // alter 2 things below
   //CRP= 0;
   Dispatch(pcb[0].TF_ptr);    // to dispatch/run CRP pcb[CRP].TF_ptr
     // Kernel();      // for kernel control


   return 0;
}

void Kernel(TF_t *TF_ptr) {
   int pid,i;
   char key;

   //change state in PCB of CRP to kernel mode
   pcb[CRP].mode = KMODE;
   //save TF_ptr to PCB of CRP
   pcb[CRP].TF_ptr = TF_ptr;
   
   switch(TF_ptr->intr_num)
   {
      case TIMER_INTR:
         TimerISR(); 
         break;
      default:
         printf("Non-magical");
   }
   
   
   //call TimerISR() to service timer interrupt as it just occurred
   TimerISR();
   if (cons_kbhit()) {
      key = cons_getchar(); // key = cons_getchar();
      switch(key) {
         case 'n':                                                   //if 'n'
            if (none_q.size == 0){                                   //no processes left in none queue
               cons_printf("No more process!\n");                    //"No more process!\n" (msg on target PC)
            }else{
            pid = DeQ(&none_q);                                      //get 1st PID un-used (dequeue none queue)
            CreateISR(pid);                                          //call CreateISR() with it to create new process
            i = 1;
            for(i  ; i<Q_SIZE;i++){
            
               
            }
               
            }
            break;
         case 't':                                                   //if 't'
           
            TerminateISR();                                          //call TerminateISR() to terminate CRP
            break;   
         case 'b':                                                   //if 'b'
           
            breakpoint();                                            // this goes back to GDB prompt
            break;
         case 'q':                                                   //if 'q'
            
            exit(0);                                                 //just do exit(0);
      }                                                              // end switch
   }                                                                 // end if some key pressed
  
   SelectCRP();                                                       //call SelectCRP() to settle/determine for next CRP
   Dispatch(pcb[CRP].TF_ptr);
}

