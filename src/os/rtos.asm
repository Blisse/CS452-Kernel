.globl Create
Create:
    THESE ARE WRONG
    swi 0

.globl MyTid
MyTid:
    swi 1

.globl MyParentTid
MyParentTid:
    swi 2

.globl Pass
Pass:
    swi 3

.globl Exit
Exit:
    swi 4
