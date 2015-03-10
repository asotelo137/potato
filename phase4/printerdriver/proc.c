// proc.c, 159
// processes are here

#include "spede.h"   // for IO_DELAY() needed here below
#include "extern.h"  // for current_run_pid needed here below
#include "proc.h"    // for Idle, SimpleProc, DispatchProc
#include "syscall.h"

void Idle() {
   int i;//seconds;
   for(;;){
   cons_printf("0");//print 0 on PC
   //seconds= 4 - (CRP%4);
   //Sleep(seconds);
   for(i=0; i<1666000; i++) IO_DELAY();//busy-loop delay for about 1 sec
   } 
}

void UserProc() {
  // int i;
   int seconds;
   for(;;){
   cons_printf("%d",CRP);//print its pid (CRP) on PC
   seconds= 4 - (CRP%4);
   Sleep(seconds);
   //printf("seconds %d", seconds);
   // Sleep(seconds);
   //for(i=0; i<1666000; i++) IO_DELAY();//busy-loop delay for about 1 sec
   }
   
}

//Phase 3 ******************************************************************8
void Producer() {
   int i,spid;//,seconds;
   spid=GetPid();
   while(1){
      //seconds= 4 - (i%4);
      SemWait(product_semaphore);// Wait for product semaphore
      cons_printf("Proc %d is producing... ",spid);
      product += 100;
      cons_printf("+++ product is now %d \n",product);
      SemPost(product_semaphore);// post product semaphore
      for(i=0; i<1666000; i++) IO_DELAY();
      //Sleep(seconds);
   }
}
void Consumer() {
   int i,spid;//,seconds;
   spid=GetPid();
   while(1){
      
     // seconds= 4 - (i%4);
      SemWait(product_semaphore);// Wait for product semaphore
      cons_printf("Proc %d is consuming... ",spid);
      product -= 100;
      cons_printf("--- product is now %d \n",product);
      SemPost(product_semaphore);// post product semaphore
      
      for(i=0; i<1666000; i++) IO_DELAY();
     // Sleep(seconds);
   }
}
 
//Phase 4 ******************************************************************8

void PrintDriver(){
   // send ch to data reg and control code to control reg
   // then repeatedly poll for printer ACK unless timed out
   // #include <spede/machine/parallel.h> // flag constants used below
   //int TIME_OUT=3*1666000; // time out 3 secs
   int i, code,TIME_OUT_COUNT,pid;
   char str []= "Hello, my team is called PotatoOS!\n It's time to bake potato!\n\0";
   char *p;
   
   print_semaphore = SemGet(0); // should it be -1? depends on IRQISR();
   TIME_OUT_COUNT =0;
   
   // make sure printer's powered up, cable connected, the following
   // statements (until the Sleep call) will reset the printer and the
   // first IRQ 7 will occur
   outportb(LPT1_BASE+LPT_CONTROL, PC_SLCTIN); // Printer Control, SeLeCT INterrupt
   code = inportb(LPT1_BASE+LPT_STATUS);
   for(i=0; i<50; i++) IO_DELAY();             // needs delay
   outportb(LPT1_BASE+LPT_CONTROL, PC_INIT|PC_SLCTIN|PC_IRQEN); // IRQ ENable
   Sleep(1);                                   // printer needs time to reset

   while(1) {
      pid = GetPid();
      cons_printf("PID is %d.",pid);// my PID
      Sleep(1);  //Sleep for 1 second
      if(print_it == 1){// print_it is 1 (set by Kernel() when key polled is 'p') {
         p = str;//p copy from str
         while (p != 0 ){  //what p points to is not 0 {
            // code sending the character to the port (see above)
                     
            outportb(LPT1_BASE+LPT_DATA, ch);      // send char to data reg
            code = inportb(LPT1_BASE+LPT_CONTROL); // read control reg
            outportb(LPT1_BASE+LPT_CONTROL, code|PC_STROBE); // send with added strobe
            for(i=0; i<20; i++) IO_DELAY();        // delay for EPSON LP-571 printer
            outportb(LPT1_BASE+LPT_CONTROL, code); // send original control code
            
            
            // code busy-poll for printer readiness (see above)
            // ----------------- BUSY-POLL PSEUDO CODE BELOW ------------------
            // if interrupt-driven mode below is replaced by a semaphore wait
            // loop 3*1666000 (3 seconds) {
            /*
            while(TIME_OUT_COUNT != TIME_OUT){
               //    code = PS_ACK & inportb(LPT1_BASE+LPT_STATUS); // read status
               code = PS_ACK & inportb(LPT1_BASE+LPT_STATUS); // read status
               //    if(code == 0) break;    // printer ACK'ed
               if(code == 0) break;    // printer ACK'ed
               //    IO_DELAY();             // or, wait 0.65us, try again
               IO_DELAY();             // or, wait 0.65us, try again
               // }
               TIME_COUNT_OUT++;   
            }
            // if the loop count did get to that many (timed out) 
            if ( TIME_OUT_COUNT == TIME_OUT ){
               //    show msg: printer timed out. Check printer!
               cons_printf("printer timed out. Check printer!\n");
               // }
            }
            // or, do a semaphore wait (for interrupt-driven mode)
            */
            //increment pointer p (to point to the next character)
            SemWait(print_semaphore);
            p++;
         } // while what p...
         
      } // if print_it...
     if (print_it ==1)
            print_it = 0; //set print_it back to 0
   } // while(1)
   
} // PrintDriver()
   


//phase 4 *********************************************************
void IRQ7(){
   int pid;
   
   outportb(0x20,0x67);
   if(semaphore[print_semaphore].wait_q.size>0){
      pid = DeQ(&semaphore[print_semaphore].wait_q);
      EnQ(pid,&run_q);
      pcb[pid].state=RUN;
   }
}
