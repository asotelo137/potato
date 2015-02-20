// proc.c, 159
// processes are here

#include "spede.h"   // for IO_DELAY() needed here below
#include "extern.h"  // for current_run_pid needed here below
#include "proc.h"    // for Idle, SimpleProc, DispatchProc

-void Dispatch() {
-   if(CRP == 0) {//if CRP is 0:
-    Idle();  //call Idle() as proc run
-   }else //(all other processes)
-      //call UserProc() as proc run
-      UserProc();
-   
-}

void Idle() {
   cons_printf("0");//print 0 on PC
   for(;;){ 
      IO_DELAY();//busy-loop delay for about 1 sec
   }
}

void UserProc() {
   cons_printf("%d",CRP);//print its pid (CRP) on PC
   for(;;){ 
      IO_DELAY();//busy-loop delay for about 1 sec
   }
}
