// entry.S, 159
// entry points of interrupts

#include <spede/machine/asmacros.h>
#include "entry.h"

// set DS and ES to Kernels data segment
#define SET_KSEGS movw $(KDATA), %ax; mov %ax, %ds; mov %ax, %es

.comm kstack, KSTACK_SIZE, 1 // define kernel stack space
.text

// Dispatch() loads registers from trapframe to run
ENTRY(Dispatch)
   movl 4(%esp), %eax   // load stack pointer from eax
   movl %eax, %esp
   popl %gs
   popl %fs
   popl %es
   popl %ds
   popa                 // restore general registers
   add $4, %esp         // skip 4 bytes that stored intr_num
   iret

// get pid entry
ENTRY(SleepEntry)
   pushl $SLEEP_INTR
   jmp IntoKernel
   
//Sleep Entry
ENTRY(GetPidEntry)
   pushl $GETPID_INTR
   jmp IntoKernel
   
// push intr type number then jump to common handler
ENTRY(TimerEntry)
   pushl $TIMER_INTR
   jmp IntoKernel
   
// push intr type number thn jump
ENTRY(SemWaitEntry)
   pushl $SEMWAIT_INTR
   jmp IntoKernel
   
//push 
ENTRY(SemPostEntry)
   pushl $SEMPOST_INTR
   jmp IntoKernel
   
//Phase 4
ENTRY(IRQ7Entry)
   pushl $IRQ7_INTR
   jmp IntoKernel
   
ENTRY(SemGetEntry)
   pushl $SEMGET_INTR
   jmp IntoKernel
   
//Phase 5
ENTRY(MsgSndEntry)
   pushl $MSGSND_INTR
   jmp IntoKernel
   
   //line 63   
   
ENTRY(MsgRcvEntry)
   pushl $MSGRCV_INTR
   jmp IntoKernel
// line 67

// Common IRQ entry, save context and call Kmain()
IntoKernel:
   pusha
   pushl %ds
   pushl %es
   pushl %fs
   pushl %gs
   movl %esp, %edx
   cld
   SET_KSEGS      // set DS and ES to kernels data segments
   leal kstack + KSTACK_SIZE, %esp
   pushl %edx
   call CNAME(Kernel)
