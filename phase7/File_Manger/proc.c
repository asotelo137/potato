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
      cons_printf(" %d ", GetPid());
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
void Shell(){
   int BAUD_RATE, divisor; //for serial oal port 
   msg_t msg;              //local message space
   char login[101], password[101],command[101]; //login and password strings
   int STDIN = 4, STDOUT = 5;
   int result;
   MyBZero((char *) &terminal.TX_q,sizeof(q_t));
   MyBZero((char *) &terminal.RX_q,sizeof(q_t));
   MyBZero((char *) &terminal.echo_q,sizeof(q_t));//clear 3 queues: TX_q, RX_q, echo_q
   terminal.TX_sem = SemGet(Q_SIZE);   //get a semaphore to set TX_sem, count Q_SIZE (char space to terminal video)
   terminal.RX_sem = SemGet(0);   //get a semaphore to set RX_sem, count 0 (no char from terminal KB)
   terminal.echo = 1;   //set echo to 1 (default is to echo back whatever typed from terminal)
   terminal.TX_extra = 1;   //set TX_extra to 1 (an IRQ3 TXRDY event missed)
   
   /*
   // COM1-8_IOBASE: 0x3f8 0x2f8 0x3e8 0x2e8 0x2f0 0x3e0 0x2e0 0x260
   // transmit speed 9600 bauds, clear IER, start TXRDY and RXRDY
   // Data communication acronyms:
   //    IIR Intr Indicator Reg
   //    IER Intr Enable Reg
   //    ETXRDY Enable Xmit Ready
   //    ERXRDY Enable Recv Ready
   //    MSR Modem Status Reg
   //    MCR Modem Control Reg
   //    LSR Line Status Reg
   //    CFCR Char Format Control Reg
   //    LSR_TSRE Line Status Reg, Xmit+Shift Regs Empty
   */
   // set baud rate 9600
   BAUD_RATE = 9600;              // Mr. Baud invented this
   divisor = 115200 / BAUD_RATE;  // time period of each baud
   outportb(COM2_IOBASE+CFCR, CFCR_DLAB);          // CFCR_DLAB 0x80
   outportb(COM2_IOBASE+BAUDLO, LOBYTE(divisor));
   outportb(COM2_IOBASE+BAUDHI, HIBYTE(divisor));
   // set CFCR: 7-E-1 (7 data bits, even parity, 1 stop bit)
   outportb(COM2_IOBASE+CFCR, CFCR_PEVEN|CFCR_PENAB|CFCR_7BITS);
   outportb(COM2_IOBASE+IER, 0);
   // raise DTR, RTS of the serial port to start read/write
   outportb(COM2_IOBASE+MCR, MCR_DTR|MCR_RTS|MCR_IENABLE);
   IO_DELAY();
   outportb(COM2_IOBASE+IER, IER_ERXRDY|IER_ETXRDY); // enable TX, RX events
   IO_DELAY();

   
  while(1){// infinite loop:
   
      while(1){//loop A:
         //prompt valid commands (send msg to STDOUT, receive reply)
         MyStrCpy(msg.data,"available commands: whoami, bye \n\0");
         msg.recipient = STDOUT;
         MsgSnd(&msg);
         MsgRcv(&msg);
         //prompt for login (send msg to STDOUT, receive reply)
         MyStrCpy(msg.data,"login: \0");
         msg.recipient=STDOUT;
         MsgSnd(&msg);
         MsgRcv(&msg);
         //get login entered (send msg to STDIN, receive reply)
         msg.recipient=STDIN;
         MsgSnd( &msg);
         MsgRcv(&msg);
         MyStrCpy(login,msg.data);
         //prompt for password (same as above)
         MyStrCpy(msg.data,"password: \0");
         msg.recipient=STDOUT;
         MsgSnd(&msg);
         MsgRcv(&msg);
         //get password entered (same as above)
         msg.recipient=STDIN;
         MsgSnd( &msg);
         MsgRcv(&msg);
         MyStrCpy(password,msg.data);
         //string-compare login and password; if same, break loop A
         //(else) prompt "Invalid login!\n\0"
          result = MyStrcmp(login,password); 
        if( result){
           break;
        }else
         if(result == 0 ){
            MyStrCpy(msg.data," Invalid login! \n\0");
            msg.recipient=STDOUT;
            MsgSnd(&msg);
            MsgRcv(&msg);
         }
      }//repeat loop A
      while(1){//loop B:
         //prompt for entering command string
         MyStrCpy(msg.data,"enter command: \0");
         msg.recipient=STDOUT;
         MsgSnd(&msg);
         MsgRcv(&msg);  
         //get command string entered
         msg.recipient=STDIN;
         MsgSnd(&msg);
         MsgRcv(&msg);
         //MyStrCpy(command,msg.data);
         if (MyStrlen(msg.data) == 0 ) {
            continue;//continue (loop B)
         }else if(MyStrcmp(msg.data,"bye")){//if command string is "bye" {
            break;//break (loop B)
         }else if(MyStrcmp(msg.data,"whoami")){//if command string is "whoami" {
            //show login string,
            MyStrCpy(msg.data,login);
            msg.recipient=STDOUT;
            MsgSnd(&msg);
            MsgRcv(&msg);
            //and an additional "\n\0" (for aesthetics)
            MyStrCpy(msg.data,"\n\0");
            msg.recipient=STDOUT;
            MsgSnd(&msg);
            MsgRcv(&msg);
            continue;//continue (loop B)
         }
         else{//other strings {
            MyStrCpy(msg.data,"Command not found!\n\0");
            msg.recipient=STDOUT;
            MsgSnd(&msg);
            MsgRcv(&msg);        //show "Command not found!\n\0"
         }//}
     
         
      }//repeat loop B
   }//repeat infinite loop*/
}

void STDIN(){
   char *p , ch;
   msg_t msg;
   while(1){//infinite loop:
   //receive msg
      //MsgSnd(STDIN, &msg);
      MsgRcv(&msg);
   //char ptr p points to msg.data
      p = msg.data;

      while(1) {//loop A:
         SemWait(terminal.RX_sem);//semaphore wait on RX_sem
         ch = DeQ(&terminal.RX_q);//ch = dequeue from RX_q
         if(ch == '\r'){//if ch is '\r', break loop A  // CR (Carriage Return) ends string
         break;
         }
         //*p++=ch ;
         *p = ch;
         p++;
        // ch=p;
      }//repeat loop A
      *p = '\0';   // add NUL to terminate msg.data
      
      msg.recipient = msg.sender; //set msg recipient with sender
      MsgSnd(&msg);// send msg, as reply to sender (MsgSndISR() must authenticate sender)
   }//repeat infinite loop*/

}

void STDOUT(){
   msg_t msg;
   char *p ;
   //STDOUT() does:
   while(1){//infinite loop:
      MsgRcv(&msg);//receive msg
      p = msg.data;//char ptr p points to msg data

      while(*p != '\0'){//loop A (until p points to null):
         SemWait(terminal.TX_sem);//semaphore-wait on TX_sem of terminal interface
         EnQ(*p,&terminal.TX_q);//enqueue what p points to to TX_q of terminal interface
         TipIRQ3();//issue syscall "TipIRQ3();" to manually start IRQ 3
         if(*p == '\n'){//if what p points to is '\n' {
            SemWait(terminal.TX_sem);//semaphore-wait on TX_sem of terminal interface
            *p='\r';
            EnQ(*p,&terminal.TX_q);//enqueue '\r' to TX_q  of terminal interface
         }
         p++;//advance p
      }//repeat loop A

      msg.recipient = msg.sender;//prep msg: set recipient with sender
      MsgSnd(&msg);//send msg (back to sender)
   }//repeat infinite loop
}
