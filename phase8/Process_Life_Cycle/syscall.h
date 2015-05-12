// syscall.h

#ifndef _SYSCALL_H_
#define _SYSCALL_H_
#include "type.h"
///Phase 5///
//#define MSGSND 53
//#define MSGRCV 54
/////////////

int GetPid();      // no input, 1 return
void Sleep(int);   // 1 input, no return
void SemWait(int);
void SemPost(int);
// phase 4 printing
int SemGet(int);
//phase 5
void MsgSnd(msg_t *);
void MsgRcv(msg_t *);
//phase6
void TipIRQ3();
//phase 8 
void Fork(char *); // argument is addr of executable
int Wait(int *);   // returns PID of exited child, argument is to get exit #
void Exit(int);    // argument is exit number to return to parent

#endif
