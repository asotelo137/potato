// proc.c, 159
// processes are here

#include "spede.h"   // for IO_DELAY() needed here below
#include "extern.h"  // for current_run_pid needed here below
#include "proc.h"    // for Idle, SimpleProc, DispatchProc
#include "syscall.h"
#include "tool.h"

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

void PrintDriver() {
   int i, code;
   char str[] = "Hello, my team is called PotatoOS!\nIt's time to bake potato!\n\0";
   char *p;

   print_semaphore = SemGet(-1); // should it be -1? depends on IRQISR()

// make sure printer's powered up, cable connected, the following
// statements (until the Sleep call) will reset the printer and the
// first IRQ 7 will occur
   outportb(LPT1_BASE+LPT_CONTROL, PC_SLCTIN); // Printer Control, SeLeCT INterrupt
   code = inportb(LPT1_BASE+LPT_STATUS);
   for(i=0; i<50; i++) IO_DELAY();             // needs delay
   outportb(LPT1_BASE+LPT_CONTROL, PC_INIT|PC_SLCTIN|PC_IRQEN); // IRQ ENable
   Sleep(1);                                   // printer needs time to reset

   while(1) {
      cons_printf("My pid is : %d\n", GetPid());
      Sleep(1);
      if (print_it == 1 ){//(set by Kernel() when key polled is 'p') {
         p = str;
         while (*p){//what p points to is not 0 {

            // code sending the character to the port (see above)
            outportb(LPT1_BASE+LPT_DATA, *p);      // send char to data reg
            code = inportb(LPT1_BASE+LPT_CONTROL); // read control reg
            outportb(LPT1_BASE+LPT_CONTROL, code|PC_STROBE); // send with added strobe
            for(i=0; i<50; i++) IO_DELAY();        // delay for EPSON LP-571 printer
            outportb(LPT1_BASE+LPT_CONTROL, code); // send original control code
            // code busy-poll for printer readiness (see above)
            // or, do a semaphore wait (for interrupt-driven mode)
            SemWait(print_semaphore);
            p++;//increment pointer p (to point to the next character)
         } // while what p...
      } // if print_it...
      print_it = 0;
   } // while(1)
} // PrintDriver()
   
