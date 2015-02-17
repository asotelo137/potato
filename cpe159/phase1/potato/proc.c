// proc.c, 159
// processes are here

#include "spede.h"   // for IO_DELAY() needed here below
#include "extern.h"  // for current_run_pid needed here below
#include "proc.h"    // for Idle, SimpleProc, DispatchProc

void Dispatch() {
   if CRP is 0:
      call Idle() as proc run
   else (all other processes)
      call UserProc() as proc run
}

void Idle() {
   print 0 on PC
   busy-loop delay for about 1 sec
}

void UserProc() {
   print its pid (CRP) on PC
   busy-loop delay for about 1 sec
}
