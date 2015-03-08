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
   int i;
   int seconds;
   for(;;){
   i=GetPid();
   cons_printf("%d",i);//print its pid (CRP) on PC
   seconds= 4 - (i%4);
   Sleep(seconds);
   //printf("seconds %d", seconds);
   // Sleep(seconds);
   //for(i=0; i<1666000; i++) IO_DELAY();//busy-loop delay for about 1 sec
   }
   
}
 
void Producer(){
   int i;
   while(1){
      SemWait(product_semaphore);// Wait for product semaphore
      cons_printf("Proc %d is producing... ",CRP);
      product += 100;
      cons_printf("+++ product is now %d \n",product);
      SemPost(product_semaphore);// post product semaphore
      for(i=0; i<1666000; i++) IO_DELAY();
   }
}

void Consumer(){
   int i;
   while(1){
      SemWait(product_semaphore);// Wait for product semaphore
      cons_printf("Proc %d is consuming... ",CRP);
      product -= 100;
      cons_printf("--- product is now %d \n",product);
      SemPost(product_semaphore);// post product semaphore
      for(i=0; i<1666000; i++) IO_DELAY();
   }
}
