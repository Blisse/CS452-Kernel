.globl GetR1
GetR1:
    mov r0, r1
    bx lr

.globl GetR2
GetR2:
    mov r0, r2
    bx lr

.globl GetR3
GetR3:
    mov r0, r3
    bx lr

.globl GetR4
GetR4:
    mov r0, r4
    bx lr

.globl GetR5
GetR5:
    mov r0, r5
    bx lr

.globl GetR6
GetR6:
    mov r0, r6
    bx lr

.globl GetR7
GetR7:
    mov r0, r7
    bx lr

.globl GetR8
GetR8:
    mov r0, r8
    bx lr

.globl GetR9
GetR9:
    mov r0, r9
    bx lr

.globl GetR10
GetR10:
    mov r0, r10
    bx lr

.globl GetR11
GetR11:
    mov r0, r11
    bx lr

.globl GetR12
GetR12:
    mov r0, r12
    bx lr

.globl GetSP
GetSP:
    mov r0, sp
    bx lr

.globl GetUserSP
GetUserSP:
    msr cpsr_c, #0xDF
    mov r0, sp
    msr cpsr_c, #0xD3
    bx lr

.globl GetLR
GetLR:
    mov r0, lr
    bx lr

.globl GetPC
GetPC:
    mov r0, pc
    bx lr

.globl GetCPSR
GetCPSR:
    mrs r0, cpsr
    bx lr

.globl GetSPSR
GetSPSR:
    mrs r0, spsr
    bx lr
