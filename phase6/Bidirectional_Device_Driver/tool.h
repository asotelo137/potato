// tool.h, 159

#ifndef _TOOL_H
#define _TOOL_H

#include "type.h" // q_t needs be defined in code below

void MyBZero(char *, int );
int DeQ(q_t *);
void EnQ(int, q_t *);
void MsgEnQ(msg_t *p, msg_q_t *q);
msg_t *MsgDeQ(msg_q_t *p);
void MyStrCpy(char *dest, char *src);

#endif

