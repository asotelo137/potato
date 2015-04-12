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

#endif
