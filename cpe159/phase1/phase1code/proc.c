// proc.c, 159
// processes are here

#include "spede.h"   // for IO_DELAY() needed here below
#include "extern.h"  // for current_run_pid needed here below
#include "proc.h"    // for Idle, SimpleProc, DispatchProc


void Idle() {
   int tick;
   while(1){
   cons_printf("0");//print 0 on PC
      for(tick= 0 ;tick <16660000;tick++){ 
         IO_DELAY();//busy-loop delay for about 1 sec
      }
   }
}

void UserProc() {
   int tick;
   while(1){
      cons_printf("%d",CRP);//print its pid (CRP) on PC
      for(tick =0 ;tick < 16660000;tick++){ 
         IO_DELAY();//busy-loop delay for about 1 sec
      }
   }
}
