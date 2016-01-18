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
