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

// kernel data structure:
int CRP, sys_time;                // current running PID, -1 means no process
int product_semaphore,product,semaphoreID;
q_t run_q, none_q,sleep_q,semaphore_q;      // processes ready to run and not used, asleep and semaphores
pcb_t pcb[MAX_PROC];    // process table
char stack[MAX_PROC][STACK_SIZE]; // run-time stacks for processes
//(include stuff from timer lab and new PCB described in 1.html)
semaphore_t semaphore;
struct i386_gate *IDT_ptr;

void SetEntry(int entry_num, func_ptr_t func_ptr){
   struct i386_gate *gateptr = &IDT_ptr[entry_num];
   fill_gate(gateptr,(int)func_ptr,get_cs(),ACC_INTR_GATE,0);
}

void InitIDT(){
   IDT_ptr = get_idt_base();//locate IDT
   cons_printf("IDT is at %u. \n",IDT_ptr);
   SetEntry(32,TimerEntry);//prime IDT Entry
   SetEntry(48,GetPidEntry);
   SetEntry(49,SleepEntry);
   SetEntry(50,SemWaitEntry);
   SetEntry(51,SemPostEntry);
   outportb(0x21,~1);
}



void InitData() {
   int i;
   sys_time = 0;
   
   MyBZero((char *)&run_q,0);
   MyBZero((char *)&none_q,0);
   MyBZero((char *)&sleep_q,0);
   MyBZero((char *)&semaphore_q,0);
   
   for(i = 1 ; i<Q_SIZE;i++){
      pcb[i].state = NONE;
      EnQ(i,&none_q);
      EnQ(i,&semaphore_q);
   }
   product = 0;
   product_semaphore = DeQ(&semaphore_q);
   CRP = 0;
}

void SelectCRP() {       // select which PID to be new CRP
   if(CRP > 0){
      return;
   }    
   //if it's' 0 (Idle), change its state in PCB to RUN
   if(CRP == 0){
      pcb[0].state = RUN;
   }

   if(run_q.size == 0 ){
      CRP = 0;
   }else{  
      CRP = DeQ(&run_q); 
      pcb[CRP].mode = UMODE;
      pcb[CRP].state = RUNNING;
   }
}
int main() {
   InitData(); 		//call Init Data to initialize kernel data
   CreateISR(0);	//call CreateISR(0) to create Idle process (PID 0)
   InitIDT();
   cons_printf("{pcb[0] is at %u. \n",pcb[0].TF_ptr);
   Dispatch(pcb[0].TF_ptr);    // to dispatch/run CRP
   
   return 0;
}
void Kernel(TF_t *TF_ptr) {

   int pid,i;
   char key;
   
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
      case SLEEP_INTR:
         SleepISR(TF_ptr->ebx);
         break;
      case SEMWAIT_INTR:
         SemWaitISR();
         break;
      case SEMPOST_INTR:
         SemPostISR();
         break;
      default:
         cons_printf("Panic!\n");
         breakpoint();
         break;
   }
   
   if (cons_kbhit()) {
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
           
            for(i =1; i<Q_SIZE;i++){
//               printf("%d is in runque %d\n",run_q.q[i],i);
               
            }
               
            }
            break;
         case 't': TerminateISR(); break;   
         case 'b':                                                   //if 'b'
           // printf("b pressed \n");
            breakpoint();                                            // this goes back to GDB prompt
            break;
         case 'w':
            
           // for(i =1; i<Q_SIZE;i++){
            //   printf("run_q %d is %d,none_q %d is %d \n" , i , run_q.q[i], i , none_q.q[i]);
           // }
            break;
         case 'q':                                                   //if 'q'
            //printf("q pressed\n");
            exit(0);                                                 //just do exit(0);
      }                                                              // end switch
   }                                                                 // end if some key pressed
//   printf("after case statement \n");
   SelectCRP();    //call SelectCRP() to settle/determine for next CRP
   Dispatch(pcb[CRP].TF_ptr);
}

