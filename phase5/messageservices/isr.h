// isr.h, 159

#ifndef _ISR_H_
#define _ISR_H_

void CreateISR(int);
void TerminateISR();
void TimerISR();
void GetPidISR();
void SleepISR(int);
void SemWaitISR();
void SemPostISR();
void IRQ7ISR();
void SemGetISR();
void MsgSndISR();
void MsgRcvISR();
#endif
