.globl Create
Create:
    swi 0
    bx lr

.globl MyTid
MyTid:
    swi 1
    bx lr

.globl MyParentTid
MyParentTid:
    swi 2
    bx lr

.globl Pass
Pass:
    swi 3
    bx lr

.globl Exit
Exit:
    swi 4
    bx lr

.globl Send
Send:
    swi 5
    bx lr

.globl Receive
Receive:
    swi 6
    bx lr

.globl Reply
Reply:
    swi 7
    bx lr
