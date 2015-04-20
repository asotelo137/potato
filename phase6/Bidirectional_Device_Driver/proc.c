// proc.c, 159
// processes are here

#include "spede.h"   // for IO_DELAY() needed here below
#include "extern.h"  // for current_run_pid needed here below
#include "proc.h"    // for Idle, SimpleProc, DispatchProc
#include "syscall.h"
#include "tool.h"

void Idle() {
   int i;//seconds;
   for(;;){
   cons_printf(" 0 ");//print 0 on PC
   //seconds= 4 - (CRP%4);
   //Sleep(seconds);
   for(i=0; i<1666000; i++) IO_DELAY();//busy-loop delay for about 1 sec
   } 
}

void UserProc() {
  // int i;
   int seconds;
   for(;;){
      cons_printf("%d ",CRP);//print its pid (CRP) on PC
      seconds= 4 - (CRP%4);
      Sleep(seconds);
      //printf("seconds %d", seconds);
      // Sleep(seconds);
      //for(i=0; i<1666000; i++) IO_DELAY();//busy-loop delay for about 1 sec
   }
   
}

//Phase 3 ******************************************************************8
void Producer() {
   int i,spid;//,seconds;
   spid=GetPid();
   while(1){
      //seconds= 4 - (i%4);
      SemWait(product_semaphore);// Wait for product semaphore
      cons_printf("Proc %d is producing... ",spid);
      product += 100;
      cons_printf("+++ product is now %d \n",product);
      SemPost(product_semaphore);// post product semaphore
      for(i=0; i<1666000; i++) IO_DELAY();
      //Sleep(seconds);
   }
}
void Consumer() {
   int i,spid;//,seconds;
   spid=GetPid();
   while(1){
      
     // seconds= 4 - (i%4);
      SemWait(product_semaphore);// Wait for product semaphore
      cons_printf("Proc %d is consuming... ",spid);
      product -= 100;
      cons_printf("--- product is now %d \n",product);
      SemPost(product_semaphore);// post product semaphore
      
      for(i=0; i<1666000; i++) IO_DELAY();
     // Sleep(seconds);
   }
}
 
//Phase 4 ******************************************************************8

void PrintDriver() {
   int i, code;
   //char str[] = "Hello, my team is called PotatoOS!\nIt's time to bake potato!\n\0";
   msg_t my_msg;
   char *p;

   print_semaphore = SemGet(-1); // should it be -1? depends on IRQISR()

// make sure printer's powered up, cable connected, the following
// statements (until the Sleep call) will reset the printer and the
// first IRQ 7 will occur
   outportb(LPT1_BASE+LPT_CONTROL, PC_SLCTIN); // Printer Control, SeLeCT INterrupt
   code = inportb(LPT1_BASE+LPT_STATUS);
   for(i=0; i<50; i++) IO_DELAY();             // needs delay
   outportb(LPT1_BASE+LPT_CONTROL, PC_INIT|PC_SLCTIN|PC_IRQEN); // IRQ ENable
   Sleep(1);                                   // printer needs time to reset

   while(1) {
      cons_printf("My pid is : %d\n", GetPid());
      //Sleep(1);
      //phase 5
      MsgRcv(&my_msg);
      // if (print_it == 1 ){//(set by Kernel() when key polled is 'p') {
      p = my_msg.data;
      while (*p){//what p points to is not 0 {

         // code sending the character to the port (see above)
         outportb(LPT1_BASE+LPT_DATA, *p);      // send char to data reg
         code = inportb(LPT1_BASE+LPT_CONTROL); // read control reg
         outportb(LPT1_BASE+LPT_CONTROL, code|PC_STROBE); // send with added strobe
         for(i=0; i<50; i++) IO_DELAY();        // delay for EPSON LP-571 printer
         outportb(LPT1_BASE+LPT_CONTROL, code); // send original control code
         // code busy-poll for printer readiness (see above)
         // or, do a semaphore wait (for interrupt-driven mode)
         SemWait(print_semaphore);
         p++;//increment pointer p (to point to the next character)
      } // while what p...
      //} // if print_it...
      //print_it = 0;
   } // while(1)
} // PrintDriver()

//phase 5 ******************************************************************************************
void Init(){
   int key;
   msg_t msg;
   char str[]= "Hello, my team is called PotatoOS!\nIt's time to bake potato!\n\0";
   msg.recipient = 2;
   MyStrCpy(msg.data, str);
   while(1){ 
      cons_printf("%d", GetPid());
      Sleep(1);
      //int i;
      //for(i=0; i<1666000; i++) IO_DELAY();//busy-loop delay for about 1 sec
      if (cons_kbhit()) {
         key = cons_getchar(); // key = cons_getchar();
         switch(key) {
            case 'b':                                                   //if 'b'
              // printf("b pressed \n");
               breakpoint();                                            // this goes back to GDB prompt
               break;
            case 'p'://phase 4 printing %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
               MsgSnd(&msg); // set global print_it to 1
               break;
            case 'q':                                                   //if 'q'
               //printf("q pressed\n");
               exit(0);                                                 //just do exit(0);
         }                                                              // end switch
      }
   }
}

//phase 6 **************************************************************************************
void shell(){
   int BAUD_RATE, divisor; //for serial oal port 
   msg_t msg;              //local message space
   char login[101], password[101]; //login and password strings
   int STDIN = 4, STDOUT = 5;
   
   /*1st initialize terminal interface data structure (below)
   then initialize serial port (below)

   infinite loop:
      loop A:
         prompt valid commands (send msg to STDOUT, receive reply)
         prompt for login (send msg to STDOUT, receive reply)
         get login entered (send msg to STDIN, receive reply)
         prompt for password (same as above)
         get password entered (same as above)
         string-compare login and password; if same, break loop A
         (else) prompt "Invalid login!\n\0"
      repeat loop A
      loop B:
         prompt for entering command string
         get command string entered
         if command string is empty {
            continue (loop B)
         }
         if command string is "bye" {
            break (loop B)
         }
         if command string is "whoami" {
            show login string,
            and an additional "\n\0" (for aesthetics)
            continue (loop B)
         }
         other strings {
            show "Command not found!\n\0"
         }
      repeat loop B
   repeat infinite loop*/
}

void STDIN(){
   /*infinite loop:
      receive msg
      char ptr p points to msg.data

      loop A:
         semaphore wait on RX_sem
         ch = dequeue from RX_q
         if ch is '\r', break loop A  // CR (Carriage Return) ends string
         *p++ = ch;
      repeat loop A

      *p = '\0';   // add NUL to terminate msg.data

      set msg recipient with sender
      send msg, as reply to sender (MsgSndISR() must authenticate sender)
   repeat infinite loop*/

}

void STDOUT(){
   /*STDOUT() does:
   infinite loop:
      receive msg
      char ptr p points to msg data

      loop A (until p points to null):
         semaphore-wait on TX_sem of terminal interface
         enqueue what p points to to TX_q of terminal interface
         issue syscall "TipIRQ3();" to manually start IRQ 3
         if what p points to is '\n' {
            semaphore-wait on TX_sem of terminal interface
            enqueue '\r' to TX_q  of terminal interface
         }
         advance p
      repeat loop A

      prep msg: set recipient with sender
      send msg (back to sender)
   repeat infinite loop
   */
}
