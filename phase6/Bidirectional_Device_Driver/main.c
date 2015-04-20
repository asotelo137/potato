// main.c, 159
// simulated kernel
//
// Team Name: Potato (Members: Aaron Sotelo and Joey Brennan)

//include statements...
#include "spede.h"      // spede stuff
#include "main.h"       // main stuff
#include "isr.h"        // ISR's
#include "tool.h"       // handy functions
#include "proc.h"       // processes such as Init()
#include "type.h"       // processes such as Init()
#include "entry.h"
#include "TF.h"

// kernel data structure:
int CRP, sys_time;                // current running PID, -1 means no process
q_t run_q, none_q,sleep_q;      // processes ready to run and not used
pcb_t pcb[MAX_PROC];    // process table
char stack[MAX_PROC][STACK_SIZE]; // run-time stacks for processes
mbox_t mbox[MAX_PROC];
//(include stuff from timer lab and new PCB described in 1.html)

//******************************************************************
//phase 3
/*
main.c/extern.h
   apply the semaphore type to declare an array of semaphores,
   "semaphore," which has the same size as a PID queue

   a semaphore queue "semaphore_q" to be primed with available
   semaphores (numbers 1 ~ 19),
    
   need for testing (Phase 3 only, after Phase, delete):
      integers "product_semaphore" and "product"
*/
semaphore_t semaphore[Q_SIZE];
q_t semaphore_q;
int product_semaphore,product;

/******************************************************************/
//******************************************************************
//phase 4

int print_semaphore,print_it;


//******************************************************************
struct i386_gate *IDT_ptr;


void SetEntry(int entry_num, func_ptr_t func_ptr){
   struct i386_gate *gateptr = &IDT_ptr[entry_num];
   fill_gate(gateptr,(int)func_ptr,get_cs(),ACC_INTR_GATE,0);
}

void InitIDT(){
   IDT_ptr = get_idt_base();//locate IDT
   cons_printf("IDT is at %u. \n",IDT_ptr);
   SetEntry(TIMER_INTR,TimerEntry);//prime IDT Entry
   SetEntry(GETPID_INTR,GetPidEntry);
   SetEntry(SLEEP_INTR,SleepEntry);
   

   //phase 3 
   /*
   InitIDT()
   fill out IDT for the new entries:
      SemWaitEntry
      SemPostEntry
   */
   SetEntry(SEMWAIT_INTR,SemWaitEntry);
   SetEntry(SEMPOST_INTR,SemPostEntry);
   
   //phase 4 
   SetEntry(SEMGET_INTR,SemGetEntry);
   SetEntry(IRQ7_INTR,IRQ7Entry);
   
   //phase 5
   SetEntry(MSGSND_INTR,MsgSndEntry);
   SetEntry(MSGRCV_INTR,MsgRcvEntry);
   
   outportb(0x21,~129);
   //***************
    //outportb(0x21,~1);
    
}

void InitData() {
   int i;
   sys_time = print_it = 0;
   
   MyBZero((char *) &run_q,sizeof(q_t));
   MyBZero((char *) &none_q,sizeof(q_t));
   MyBZero((char *) &sleep_q,sizeof(q_t));
   MyBZero((char *) &semaphore_q,sizeof(q_t));
   
   for(i = 1 ; i<Q_SIZE;i++){
      pcb[i].state = NONE;
      EnQ(i,&none_q);
      //phase 3
      EnQ(i,&semaphore_q);
      
   }
   CRP = 0;
   
   //phase 3 
   /*
   InitData()
   empty semaphore queue and fill numbers 1~19 into it

   These are for Phase 3 as testing needs, delete after Phase:
   1. dequeue semaphore queue to give to product_semaphore
   2. MyBzero the semaphore allocated
   3. set the count of the product semaphore to 1
   4. set the product to 0
   */
   // phase 3 testing
   /*product_semaphore=DeQ(&semaphore_q);
   
   MyBZero((char *) &semaphore[product_semaphore],sizeof(semaphore_t));
   
   semaphore[product_semaphore].count = 1;
   //MyBZero((char *)semaphore[product_semaphore].wait_q,Q_SIZE);
   product=0;
   */ 
   
   
}

void SelectCRP() {       // select which PID to be new CRP
   if(CRP > 0){
      return;
   }    
   //if it's' 0 (Idle), change its state in PCB to RUN
   if(CRP == 0){
      pcb[0].state = RUN;
   }

   if(run_q.size == 0 )
      CRP = 0;
   else 
      CRP = DeQ(&run_q); 
   
   pcb[CRP].mode = UMODE;
   pcb[CRP].state = RUNNING;
   
}

int main() {
   int i,j;
   InitData(); 		//call Init Data to initialize kernel data
   CreateISR(0);//call CreateISR(0) to create Idle process (PID 0)
   i=DeQ(&none_q);
   CreateISR(i);
   j=DeQ(&none_q);
   CreateISR(j);
   InitIDT();
   cons_printf("{pcb[0] is at %u. \n",pcb[0].TF_ptr);
   Dispatch(pcb[0].TF_ptr);    // to dispatch/run CRP
   
   return 0;
}

void Kernel(TF_t *TF_ptr) {

   //int pid;
   //char key;
   
   pcb[CRP].TF_ptr=TF_ptr;
   //change state in PCB of CRP to kernel mode
   pcb[CRP].mode = KMODE;
   //call TimerISR() to service timer interrupt as it just occurred
   switch(TF_ptr->intr_num){
      
      case TIMER_INTR:
         //printf("made it into kernel\n");
         TimerISR();
         break;
      case GETPID_INTR:
         GetPidISR();
         break;
      case SLEEP_INTR://
         SleepISR(TF_ptr->ebx);
         break;
      case SEMWAIT_INTR://phase 3 *********************************
         SemWaitISR();
         break;
      case SEMPOST_INTR://phase 3 *********************************
         SemPostISR();
         break;
      case SEMGET_INTR:
         SemGetISR();
         break;
      case IRQ7_INTR:
         IRQ7ISR();
         break;
      //phase 5
      case MSGSND_INTR:
         MsgSndISR();
         break;
      case MSGRCV_INTR:
         MsgRcvISR();
         break;
      default:
         cons_printf("Panic!\n");
         breakpoint();
         break;
   }
   
  /* if (cons_kbhit()) {
      
      key = cons_getchar(); // key = cons_getchar();
      switch(key) {
         case 'n':                                                   //if 'n'
         // printf("n pressed\n");
            if (none_q.size == 0){                                   //no processes left in none queue
               cons_printf("No more process!\n");                    //"No more process!\n" (msg on target PC)
            }else{
               pid = DeQ(&none_q);                                      //get 1st PID un-used (dequeue none queue)
              //  printf("after pressing n pid is %d \n",pid);
               CreateISR(pid);                                          //call CreateISR() with it to create new process
            }
            break;
         case 't': TerminateISR(); break;   
      }                                                              // end switch
   }*/                                                                 // end if some key pressed
//   printf("after case statement \n");
   SelectCRP();    //call SelectCRP() to settle/determine for next CRP
   Dispatch(pcb[CRP].TF_ptr);
}

