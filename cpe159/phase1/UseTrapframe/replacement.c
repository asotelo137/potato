// replacement.txt, 159

//********************************************************************
// in isr.c, update old CreateISR():
//********************************************************************
void CreateISR(int pid) {

// ****************************************************
// besides code in old CreateISR() still included, add:
// ****************************************************

   MyBzero(stack[pid], STACK_SIZE); // erase stack

// point to just above stack, then drop by sizeof(TF_t)
   pcb[pid].TF_ptr = (TF_t *)&stack[pid][STACK_SIZE];
   pcb[pid].TF_ptr--;

// fill out trapframe of this new proc:
   if(pid == 0)
      pcb[pid].TF_ptr->eip = (unsigned int)Idle; // Idle process
   else
      pcb[pid].TF_ptr->eip = (unsigned int)UserProc; // other new process

   pcb[pid].TF_ptr->eflags = EF_DEFAULT_VALUE | EF_INTR;
   pcb[pid].TF_ptr->cs = get_cs();
   pcb[pid].TF_ptr->ds = get_ds();
   pcb[pid].TF_ptr->es = get_es();
   pcb[pid].TF_ptr->fs = get_fs();
   pcb[pid].TF_ptr->gs = get_gs();

}

//********************************************************************
// in proc.c/proc.h:
//********************************************************************
1. Get rid of old Dispatch() in proc.c/h.
2. Change both Idle() and UserProc() to put old code in an infinite loop:
    for(;;) {
      // original code (displaying PID, busy-loop for 1s)
    }

