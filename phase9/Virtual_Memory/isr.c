// isr.c, 159

#include "spede.h"
#include "type.h"
#include "isr.h"
#include "tool.h"
#include "extern.h"
#include "proc.h"
#include "syscall.h"
#include "FileMgr.h"

int wakingID;
int wake_period;
int sleepindex,sleeppid,sleepsize;
int semaID;
void CreateISR(int pid) {
  // printf("create\n");
   if(pid !=0 ){//if pid given is not 0 (Idle), enqueue it into run queue
    //   printf("Create recieved a pid : %d \n",pid);
      EnQ(pid,&run_q);
  
      // PCB of new proc:
      pcb[pid].mode = UMODE;//mode is set to UMODE
      pcb[pid].state= RUN;//state is set to RUN
      pcb[pid].runtime = 0;// both runtime counts are reset to 0
      pcb[pid].total_runtime = 0;// both runtime counts are reset to 0
      
      }
      MyBZero((char*) &mbox[pid],sizeof(mbox_t));
      MyBZero(stack[pid], STACK_SIZE); // erase stack
      // point to just above stack, then drop by sizeof(TF_t)
      pcb[pid].TF_ptr = (TF_t *)&stack[pid][STACK_SIZE];
      pcb[pid].TF_ptr--;
      // fill out trapframe of this new proc:
      if(pid == 0){
        pcb[pid].TF_ptr->eip = (unsigned int)Idle; // Idle process
      }else if(pid == 1){
        pcb[pid].TF_ptr->eip = (unsigned int)Init; // print process
      }else if(pid == 2){
        pcb[pid].TF_ptr->eip = (unsigned int)PrintDriver;
      }else if(pid == 3){
        pcb[pid].TF_ptr->eip = (unsigned int)Shell;
      }else if(pid == 4){
        pcb[pid].TF_ptr->eip = (unsigned int)STDIN;
      }else if(pid == 5){
        pcb[pid].TF_ptr->eip = (unsigned int)STDOUT;
      }else if(pid == 6){
        pcb[pid].TF_ptr->eip = (unsigned int)FileMgr;
      }else{
        pcb[pid].TF_ptr->eip = (unsigned int)UserProc; // other new process
      }          
      /*if(pid == 0)
        pcb[pid].TF_ptr->eip = (unsigned int)Idle; // Idle process
      else if(pid ==1)
         pcb[pid].TF_ptr->eip = (unsigned int)PrintDriver; // print process
      else if(pid%2 == 0)
        pcb[pid].TF_ptr->eip = (unsigned int)Consumer;
      else if(pid%2 == 1)
        pcb[pid].TF_ptr->eip = (unsigned int);*/
      //statements set Trapframe 
      pcb[pid].TF_ptr->eflags = EF_DEFAULT_VALUE | EF_INTR;
      pcb[pid].TF_ptr->cs = get_cs();
      pcb[pid].TF_ptr->ds = get_ds();
      pcb[pid].TF_ptr->es = get_es();
      pcb[pid].TF_ptr->fs = get_fs();
      pcb[pid].TF_ptr->gs = get_gs();
      
      //phase 9
      pcb[pid].main_table = sys_main_table;
//  return 0;
}

void TerminateISR() {
  //printf("terminate\n");
   //just return if CRP is 0 or -1 (Idle or not given)
    
    if (CRP <=0 ){
      return;
    }  
   //change state of CRP to NONE
   pcb[CRP].state = NONE;
   //queue it to none queue
   EnQ(CRP,&none_q);                             //STILL ERROR HERE
   //set CRP to -1 (none)
   CRP = -1;
 //  return 0;
}    

void GetPidISR(){
    pcb[CRP].TF_ptr->ebx = CRP;
  return;
}

void TimerISR() {
  //dismiss timer intr (IRQ 0), otherwise, new intr signal from timer
  //won't be recognized by CPU since circuit uses edge-trigger flipflop
  //0x20 is PIC control reg, 0x60 dismisses IRQ 0
  outportb(0x20,0x60);
   
  // printf("TimerISR Beggineing CRP %d \n",CRP);
  
  //upcount the runtime of CRP and system time
  pcb[CRP].runtime++;
  sys_time++;
  sleepsize=sleep_q.size;
  while(sleepsize--){
    sleeppid=DeQ(&sleep_q);
    if(pcb[sleeppid].wake_time > sys_time){
      //int wakingID;
	EnQ(sleeppid,&sleep_q);
    }else {      
    	EnQ(sleeppid,&run_q);
      pcb[sleeppid].state=RUN;
      
    }
  }
   // just return if CRP is Idle (0) or less (-1)
   if (CRP <= 0  ){
      //printf("TIMER ISR CRP is %d\n", CRP);
      return;
   }

   if(pcb[CRP].runtime == TIME_LIMIT){
    
      pcb[CRP].total_runtime += TIME_LIMIT;
      pcb[CRP].runtime=0;
      pcb[CRP].state = RUN;
      EnQ(CRP,&run_q);
      CRP = -1;
    
   }
   // return 0;
}



void SleepISR(int seconds){
  wake_period= sys_time+(100*seconds);
  pcb[CRP].wake_time=wake_period;
  EnQ(CRP,&sleep_q);
  pcb[CRP].state=SLEEP;
  CRP=-1;
  return;
  
}
//phase 3***************************************************************
void SemWaitISR(){
  
  int semID;
  semID = pcb[CRP].TF_ptr->ebx;
  //breakpoint();
  if( semaphore[semID].count > 0){
   semaphore[semID].count --;
  }else {
    EnQ(CRP,&(semaphore[semID].wait_q));
    pcb[CRP].state = WAIT;
    CRP=-1;
  }

}
// phase 3 ***********************************************************
void SemPostISR(int semID){
  
  int temp;
  //int semID; 
  
  //semID = pcb[CRP].TF_ptr->ebx;
 //breakpoint();
  if( semaphore[semID].wait_q.size ==0){
    semaphore[semID].count ++;
  }else {
    
    temp = DeQ(&semaphore[semID].wait_q);
    pcb[temp].state = RUN;
    EnQ(temp,&run_q);
  }
}

// phase 4 **********************************************************
//This function will first dismiss IRQ7 and does the same function of a semaphore-post
//(on the print_semaphore) to release the waited process which PrinterDriver() so it can resume printing

void IRQ7ISR(){
   int temp;
  int semID; 
  outportb(0x20, 0x67);
  semID = pcb[CRP].TF_ptr->ebx;
 //breakpoint();
  if( semaphore[print_semaphore].wait_q.size ==0){
    semaphore[print_semaphore].count ++;
  }else {
    
    temp = DeQ(&semaphore[print_semaphore].wait_q);
    pcb[temp].state = RUN;
    EnQ(temp,&run_q);
  }
}


// phase 4 **********************************************************
// Sem get ISR
void SemGetISR(){
  
  /*to allocate a semaphore by dequeuing the available semaphore-ID queue,
  "bzero" it, set the count to what's requested accordingly,
  and the semphore ID is returned to the calling process via its trapframe.
  */
  int gcount = pcb[CRP].TF_ptr->ebx;
  /*
  changed the if conditioncheckiing for zero to negative 1 on changs advice, remeber to include the pcb line
  inside the o condition check
  
  */
  semaID=DeQ(&semaphore_q);
  if ( semaID == -1) return;
  MyBZero((char*)&semaphore[semaID], sizeof(semaphore_t));
  semaphore[semaID].count = gcount;
  
  pcb[CRP].TF_ptr->ecx = semaID;

}

//Phase 5***********
void MsgSndISR(){
  msg_t source, *destination;
  int recipient;

  source = *(msg_t *)pcb[CRP].TF_ptr->ebx;
  recipient = source.recipient;

  source.sender = CRP;
  source.time_stamp = sys_time;

  if ((mbox[recipient].wait_q).size == 0){
  	MsgEnQ(&source, &mbox[recipient].msg_q);
  }else{
  	int tmp_pid = DeQ(&(mbox[recipient].wait_q));
  	pcb[tmp_pid].state = RUN;
  	EnQ(tmp_pid, &run_q);
  	
  	destination = (msg_t *)pcb[tmp_pid].TF_ptr->ebx;
  	memcpy((char*)destination,(char*)&source, sizeof(msg_t));
  }
}

void MsgRcvISR(){
  
	msg_t *tmp,*temp2;
	if(mbox[CRP].msg_q.size == 0){
	//code to block CRP
	//move the calling process to the wait queue of the mailbox, set its state, and reset cur_pid
		EnQ(CRP, &mbox[CRP].wait_q);
		pcb[CRP].state = WAIT;
		CRP=-1;
	}else{
	// dequeue a message (get a msg_t pointer) and use it to copy to CRP's local msg space!
	// copy the 1st message to the msg locally declared in the calling process
		tmp = MsgDeQ(&mbox[CRP].msg_q);
		temp2 = (msg_t *)pcb[CRP].TF_ptr->ebx;
	//	set_cr3(pcb[CR].main_table); 		//set the MMU to start using the designated 
							//main table for runtime memory address translation.
		memcpy((char *)temp2,(char *)tmp,sizeof(msg_t));
	}
}

//phase 6 ************************************
void IRQ3ISR(){
	int event;
	outportb(0x20,0x63);//dismiss IRQ 3: use outportb() to send 0x63 to 0x20
	//read event from COM2_IOBASE+IIR (Interrupt Indicator Register)
	event = inportb(COM2_IOBASE+IIR);
      	switch(event) {
         case IIR_TXRDY://case IIR_TXRDY, call IRQ3TX(), break, (send char to terminal video)
         		IRQ3TX();
         		break;
         case IIR_RXRDY://case IIR_RXRDY, call IRQ3RX(), break, (get char from terminal KB)
	 		IRQ3RX();
	 		break;
      	}

      	if(terminal.TX_extra == 1){//if TX_extra of terminal interface is 1
        	IRQ3TX();//call IRQ3TX() to use it to TX char
      	}
}

void IRQ3TX() { // dequeue TX_q to write to port
      char ch = '\0'; // NUL, '\0'

      if(terminal.echo_q.size != 0){//if echo queue of terminal interface is not empty {
         ch =DeQ(&terminal.echo_q);//ch = dequeue from echo queue of terminal interface
      } else {
         if(terminal.TX_q.size != 0){//if TX queue of terminal interface is not empty
            ch = DeQ(&terminal.TX_q);//ch = dequeue from TX queue of terminal interface
            SemPostISR(terminal.TX_sem);//SemPostISR( TX semaphore of terminal interface )
         }
      }

      if(ch == '\0'){//if ch is 0 {
         terminal.TX_extra = 1;//TX_extra is set to 1
      } else {
         outportb(COM2_IOBASE+DATA,ch);//use outportb() to send ch to COM2_IOBASE+DATA
         terminal.TX_extra = 0;//TX_extra is cleared (to 0)
      }
	
}

void IRQ3RX() { // queue char read from port to RX and echo queues
      char ch;

      // use 127 to mask out msb (rest 7 bits in ASCII range)
      ch = inportb(COM2_IOBASE+DATA) & 0x7F;  // mask 0111 1111
      EnQ(ch , &terminal.RX_q);//enqueue ch to RX queue
      SemPostISR(terminal.RX_sem);//SemPostISR( RX semaphore of terminal interface )

      if(ch == '\r'){//if ch is '\r' {
         EnQ((int) '\r', &terminal.echo_q);//enqueue '\r' 
         EnQ((int) '\n', &terminal.echo_q);//then '\n' to echo queue of terminal interface
      } else {
         if(terminal.echo == 1){//if echo of terminal interface is 1 {
             EnQ((int) ch , &terminal.echo_q);//enqueue ch to echo queue of terminal interface
         }
      }
	
}
//Phase 8*********************************************************
void ForkISR(){
	/*//ForkISR():
	//     A. if no more PID or no RAM page available
	
	//variables for phase 9
<<<<<<< HEAD
	int new_pid, page_num, exec_addr,
        index[5],  // need 5 free page indices
        *p,        // to fill table entries
        main_table, code_table, stack_table, code_page, stack_page;
=======
	void ForkISR() {  // ebx executable, ecx child pid to return
   	int new_pid, page_num, exec_addr,
       	index[5],  // need 5 free page indices
       	*p,        // to fill table entries
       	main_table, code_table, stack_table, code_page, stack_page,
       	i;         // to count
>>>>>>> 6ba6ca5cf64eca33af2ff8c7bdc95066b11bad08
	//////////////////////////
/*	int i,child_pid;
	int avail_page = -1;
	for (i = 0; i <MAX_PROC; i++){
		if(page[i].owner == -1){
		  avail_page= i;
		  break;
		}
	}
	child_pid = DeQ(&none_q);//dequeue to get new pid
	if(child_pid==-1|| avail_page == -1){
		cons_printf(" no more PID/RAM available!\n ");//cons_printf(): "no more PID or RAM available!\n"
		pcb[CRP].TF_ptr->ecx = -1;//set CRP's TF_ptr->ecx = -1 (syscall returns -1)
		return; //(end of ISR)
	}
	//    B. set "owner" of this page to the new PID

	page[avail_page].owner= child_pid;
	//C. copy the executable into the page, use your new MyMemcpy() coded in tool.c
	
	MyMemcpy((char *)page[avail_page].addr, (char *)pcb[CRP].TF_ptr->ebx, 4096 ); 

	//D  set PCB:
	//clear runtime and total_runtime
	pcb[child_pid].runtime= 0;
	pcb[child_pid].total_runtime = 0;
	//set state to RUN
	pcb[child_pid].state = RUN;
	//set mode to UMODE
	pcb[child_pid].mode =UMODE;
	//set ppid to CRP (new thing from this Phase)
	pcb[child_pid].ppid = CRP;
	//E. build trapframe:
	//point pcb[new PID].TF_ptr to end of page (4095) - sizoeof(TF_t) + 1
	pcb[child_pid].TF_ptr = (TF_t *)(page[avail_page].addr + 4096 - sizeof(TF_t));
	//add those statements in CreateISR() to set trapframe except
	//EIP = the page addr + 128 (skip header)
	pcb[child_pid].TF_ptr->eip = (unsigned int)(page[avail_page].addr + 128);;
	pcb[child_pid].TF_ptr->eflags = EF_DEFAULT_VALUE | EF_INTR;
	pcb[child_pid].TF_ptr->cs = get_cs();
	pcb[child_pid].TF_ptr->ds = get_ds();
	pcb[child_pid].TF_ptr->es = get_es();
	pcb[child_pid].TF_ptr->fs = get_fs();
	pcb[child_pid].TF_ptr->gs = get_gs();
      
        pcb[pid].main_table = main_table; //set PCB[new PID].main_table to the new main table
      
	//F. clear mailbox
	MyBZero((char*)&mbox[child_pid],sizeof(mbox_t));
	//G. enqueue new PID to run queue
<<<<<<< HEAD
	EnQ(child_pid,&run_q);*/
	int new_pid,
	index[5],  // need 5 free page indices
	*p,        // to fill table entries
	main_table, code_table, stack_table, code_page, stack_page,
	i,j;         // to count
	
	new_pid = DeQ(&none_q);// dequeue new pid from none q
	
	if( new_pid == -1){// check if pid is available  return if none available
		cons_printf("ForkISR(): No More PID available!\n");
		pcb[CRP].TF_ptr->ecx = -1;
		return;
	}
	
j = 0;

	for (i = 0; i < MAX_PROC; i++){ // loop to scan all pages
		if(page[i].owner == -1){ //check if page is available
			page[i].owner=new_pid;// set page owner to new pid
			index[j] = i;// index[] = this page index
			j++;
			if(j == 5){ //if got enough indexes  break
				break;
			}
		}					
	}	
	if(j != 5 ){	// if didnt get 5 indices 
		cons_printf("ForkISR(): not enough memory available!\n");//print error
		EnQ(new_pid, &none_q);// recycle/return pid
		for (i = 0; i < MAX_PROC; i++){//loop through pages to return
			if(page[i].owner == new_pid){
				page[i].owner=-1;
				//index[j++] =-1;
			}					
		}	
	}
	// with five pages obtained, set these addresses (integers):
	//    main_table, code_table, stack_table, code_page, stack_page
	//       to addresses of obtained pages       
	main_table = page[index[0]].addr;
	code_table = page[index[1]].addr;
	stack_table = page[index[2]].addr;
	code_page = page[index[3]].addr;
	stack_page = page[index[4]].addr;

	MyMemcpy((char*)main_table,(char*)sys_main_table,16);
	
	//set entries 512 and 767 of main_table to addresses of 
   	//code_table and stack_table, add two flags into entries
	p=(int *) (main_table +(512*4));
	*p = code_table +0x003 ;
	p = (int *)(main_table +(767*4));
	*p = stack_table + 0x003;
	//set entry 0 of code_table to addr of code_page (plus flags)
	p=(int *)(code_table);
	*p= code_page + 0x003;
	
	//set entry 1023 of stack_table to addr of stack_page (plus flags)
	p=(int*) (stack_table + (1023*4));
	*p= stack_page +0x003;

	// copy executable to code_page (like previous phase)	
	MyMemcpy((char *)code_page, (char *)pcb[CRP].TF_ptr->ebx, 4096 ); 

	//set things in its PCB
	//set the main_table in PCB of this new process to main_table
	pcb[new_pid].main_table = main_table;
	//clear the runtime, total_runtime
	pcb[new_pid].runtime  = 0;
	pcb[new_pid].total_runtime = 0;
	//set its state
        pcb[new_pid].state= RUN;	
	//set its mode
	pcb[new_pid].mode = UMODE;	
	//set its ppid
	pcb[new_pid].ppid=CRP;
	
	//build its trapframe:
	//set TF_ptr into stack_page (towards end of page, with space for TF)
	pcb[new_pid].TF_ptr =(TF_t*) (stack_page+4032);
	
	//set eip of TF to virtual addr 2G + 128
	pcb[new_pid].TF_ptr->eip = (unsigned int)(0x80000080);	
	
	//set other things of TF the same way as before
=======
	EnQ(child_pid,&run_q);
*/
//////////////////phase 9 from the bottom

	//build its trapframe:
  	// set TF_ptr into stack_page (towards end of page, with space for TF)
	pcb[new_pid].TF_ptr = (TF *)((stack_page)+4096) - sizeof(TF_t) + 1;
	// set eip of TF to virtual addr 2G + 128
	pcb[pid].TF_ptr->eip = (unsigned int)(2G + 128);
	// set other things of TF the same way as before
>>>>>>> 6ba6ca5cf64eca33af2ff8c7bdc95066b11bad08
	pcb[new_pid].TF_ptr->eflags = EF_DEFAULT_VALUE | EF_INTR;
	pcb[new_pid].TF_ptr->cs = get_cs();
	pcb[new_pid].TF_ptr->ds = get_ds();
	pcb[new_pid].TF_ptr->es = get_es();
	pcb[new_pid].TF_ptr->fs = get_fs();
	pcb[new_pid].TF_ptr->gs = get_gs();
<<<<<<< HEAD
    	
	//set TF_ptr again, to virtual addr 3G-64
  	pcb[new_pid].TF_ptr =(TF_t*) (0xbfffffc0);
	

	// clear mailbox
	MyBZero((char*)&mbox[new_pid],sizeof(mbox_t));
	
	// enqueue new PID to run queue
	EnQ(new_pid,&run_q);
		
	
	

=======
	// set TF_ptr again, to virtual addr 3G-64
	pcb[pid].TF_ptr->eip = (unsigned int)(3G - 64);
	
	MyBZero((char*)&mbox[new_pid],sizeof(mbox_t));	// clear mailbox of this new process
	EnQ(pid, &run_q);			//enqueue the pid of this new process to run queue
>>>>>>> 6ba6ca5cf64eca33af2ff8c7bdc95066b11bad08


}

void WaitISR(){
	int i,page_num, child_exit_num, *parent_exit_num_ptr;
	
	//A. look for a ZOMBIE child
	//loop i through all PCB
	for(i = 0; i < MAX_PROC;i++){
		if(pcb[i].ppid== CRP && pcb[i].state == ZOMBIE ){//if there's a ppid matches CRP and its state is ZOMBIE
			break;//found! (its PID is i)
		}
	}	
	if(i==MAX_PROC){//B. if not found (i is too big)
		//block CRP (parent/calling process)
		//its state becomes WAIT_CHILD (new state, add to state_t)
		pcb[CRP].state= WAIT_CHILD;
		CRP=-1;//CRP becomes -1
		return; //(end of ISR)
	}
	//C. found exited child PID i, parent should be given 2 things:
	//put i into ecx of CRP's TF (for syscall Wait() to return it)
	pcb[CRP].TF_ptr->ecx=i;
	//pass the exit number from the ZOMBIE to CRP
	parent_exit_num_ptr = (int *)pcb[CRP].TF_ptr->ebx;
	child_exit_num = pcb[i].TF_ptr->ebx;
	*parent_exit_num_ptr = child_exit_num;
	
	//D. recycle resources (bring ZOMBIE to R.I.P)
	//reclaim child's 4KB page:
	//loop through pages to match the owner to child PID (i)

	pcb[i].state=NONE;//child's state becomes NONE
	EnQ(i,&none_q);//enqueue child PID (i) back to none queue

	for (page_num = 0; page_num <MAX_PROC; page_num++){
		if(page[page_num].owner == i){
			//once found, clear page (for security/privacy)
			MyBZero((char*) page[page_num].addr,4096);
			page[page_num].owner=-1;//set owner to -1 (not used)
		}
	}
}
   
void ExitISR() {
	int ppid, child_exit_num, *parent_exit_num_ptr, page_num;

	ppid = pcb[CRP].ppid;
	if( pcb[ppid].state != WAIT_CHILD){//A. if parent of CRP NOT in state WAIT_CHILD (has yet called Wait())
		pcb[CRP].state = ZOMBIE;//state of CRP becomes ZOMBIE
		CRP=-1;//CRP becomes -1;
		return;// (end of ISR)
	}
	//B. parent is waiting, release it, give it the 2 things

	pcb[ppid].state=RUN;//parent's state becomes RUN
	EnQ(ppid,&run_q);//enqueue it to run queue
	pcb[ppid].TF_ptr->ecx=CRP;//give child PID (CRP) for parent's Wait() call to continue and return

	parent_exit_num_ptr = (int *)pcb[ppid].TF_ptr->ebx;
	child_exit_num = pcb[CRP].TF_ptr->ebx;
	*parent_exit_num_ptr = child_exit_num;//pass the child (CRP) exit number to fill out parent's local exit number
	
	//C. recycle exiting CRP's resources
	//reclaim CRP's 4KB page:
	//loop through pages to match the owner to CRP
	for (page_num = 0; page_num <MAX_PROC; page_num++){
		if(page[page_num].owner == CRP){
			MyBZero((char*) page[page_num].addr,4096);//once found, clear page (for security/privacy)
			page[page_num].owner=-1;//set owner to -1 (not used)
			pcb[CRP].state=NONE;//CRP's state becomes NONE
		}
	}

	EnQ(CRP,&none_q);//enqueue CRP back to none queue
	CRP=-1;//CRP becomes -1
}
   
