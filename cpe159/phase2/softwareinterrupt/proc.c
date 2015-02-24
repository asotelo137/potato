// proc.c, 159
// processes are here

#include "spede.h"   // for IO_DELAY() needed here below
#include "extern.h"  // for current_run_pid needed here below
#include "proc.h"    // for Idle, SimpleProc, DispatchProc


void Idle() {
   int i;
   for(;;){
   cons_printf("0");//print 0 on PC
   for(i=0; i<1666000; i++) IO_DELAY();//busy-loop delay for about 1 sec
   } 
}

void UserProc() {
  // int i;
   int seconds;
   for(;;){
   cons_printf("%d",CRP);//print its pid (CRP) on PC
   seconds= 4 - (CRP%4);
   sleep(seconds);
   //for(i=0; i<1666000; i++) IO_DELAY();//busy-loop delay for about 1 sec
   }
   
}
