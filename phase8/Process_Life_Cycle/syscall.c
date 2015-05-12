// syscall.c
// collection of syscalls, i.e., API

#include "syscall.h" // prototype these below
#include "type.h"
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
int SemGet(int count) {
  int semaid;
	asm("movl %1,%%ebx ; int $52; movl %%ecx, %0;" // CPU inst
		: "=g" (semaid) // 1 output from asm()
		: "g" (count)
		: "%ebx", "%ecx" ); // push/pop before/after asm()
  return semaid;
}

//Phase 5: code MsgSnd() and MsgRcv(), they 
void MsgSnd(msg_t *msg)
{
	asm("movl %0, %%ebx; int $53" // CPU inst
	:
	: "g" ((int)msg) // no input into asm()
	: "ebx"); // push/pop before/after asm()
}

void MsgRcv(msg_t *msg){
	asm("movl %0,%%ebx ; int $54" 
		: 
		: "g" ((int) msg)
		: "%ebx"); 
}
void TipIRQ3(){
	asm("int $35");
}
void Fork(char *exe_addr){
	asm("movl %0, %%ebx; int $55"
	:
	:"g" ((int)exe_addr)
	:"%ebx");
}
void Wait(int *exit_num){
	int pid;
	asm("movl %1, %%ebx; int $56; movl %%ecx, %0;"
	:"=g" (pid)
	:"g" ((int)exit_num)
	:"%ebx","%ecx");
   	return pid;
}
void Exit(int exit_num){
	asm("movl %0, %%ebx; int $57"
	:
	:"g" (exit_num)
	:"%ebx");
}
