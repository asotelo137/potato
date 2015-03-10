// syscall.c
// collection of syscalls, i.e., API

#include "syscall.h" // prototype these below

int GetPid() {
   int pid;

   asm("int $48; movl %%ebx, %0" // CPU inst
       : "=g"  (pid)             // 1 output from asm() 
       :                         // no input into asm()
       : "%ebx");                // push/pop before/after asm()
    
   return pid;
}

void Sleep(int sec) {
    asm("movl %0, %%ebx ;int $49"
        :
        :"g" (sec)
        :"%ebx");
}
/* PHASE 3 **********************************************
syscall.c/.h
   code:
      SemWait()
      SemPost()
*/
void SemWait(int semaphoreID){
   asm("movl %0, %%ebx ;int $50"
      :
      :"g" (semaphoreID)
      :"%ebx");
}

void SemPost(int semaphoreID){
   asm("movl %0, %%ebx ;int $51"
      :
      :"g" (semaphoreID)
      :"%ebx");
}
//phase 4 printing SemGet()
int SemGet(int count){
   int semaphoreID;
   asm(" movl %%ebx,%1; int $52; movl %%ecx, %0;" // CPU inst
      : "=g" (semaphoreID) // 1 output from asm()
      : "g" (count) // 1 input into asm()
      :"%ebx","%ecx" ); // push/pop before/after asm()
   return semaphoreID;
   
}
