# MyHello.s
#
# in order to perform syscall MsgSnd(&msg), MsgRcv(&msg), need to
# first calculate where the real RAM addr of my msg is:
# use ESP - 4096 to get the base of the 4KB RAM page this program resides.
# Since $msg is virtual, at 2G+x (0x80000000+x), and the page is virtual
# at 2G (0x80000000); subtract 2G from $msg gets x. Add x to the base is
# the real RAM addr of the msg.

.text                       # code segment
.global _start              # _start is main()

_start:                     # instructions begin
  # pushl %esp               #push the stack pointer
  # pop %ebx                #pop into register ebx
  # sub $0x1000,%ebx        #subtract 4096 from it (this is the base, real addr of the page)

   #movl $msg , %ecx         #copy $msg to register ecx
   #subl $0x80000000, %ecx   #subtract 2G from it, get x (offset)

   #addl %ecx,%ebx              #add  x (offset) to ebx (base of page) -- where msg really is
   #pushl %ebx               #save a copy (push it to stack)
   #pushl %ebx               #save another copy (push it again)
   
   movl $msg,%ebx	
   int $53                   #call interrupt number 53  # MsgSnd(&msg)

  # popl %ebx                  #pop to ebx (get a copy of real msg addr)
   movl $msg, %ebx
   int $54                   #call interrupt number 54  # MsgRcv(&msg)

   movl $msg,%ecx                  #pop to ecx (get a copy, real msg addr)
   movl 8(%ecx),%ebx          #copy time stamp (base ecx + offset of time stamp) to ebx
   int $57                    #call interrupt number 57  # Exit(time stamp)

.data                       # data segment follows code segment in RAM
msg:                        # my msg
     .long 0                # msg.sender
     .long 5                # msg.recipient
     .long 0                # msg.time_stamp
     .ascii "Hello from Team Potato\n" #msg.data (16 char)
     .rept 78                #101-23 = 78
        .ascii "\0"             #null chars
     .endr                   #end repeat
     .long 0                 #msg.code
     .long 0                 #msg.code
     .long 0                 #msg.code
     .long 0                 #msg.code

