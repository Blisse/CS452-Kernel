.globl InstallSwiHandler
InstallSwiHandler:
    mov r0, #0x00000028 @ Location of software interrupt
    ldr r1, =SwiHandler @ Address of software interrupt handler
    str r1, [r0] @ Install handler to interrupt controller
    bx lr

.globl SwiHandler
SwiHandler:
    stmfd sp!,{r1-r12,lr}
    bl print
    mov r0, #5
    ldmfd sp!,{r1-r12,pc}^

.globl Test
Test:
    stmfd sp!,{lr}
    swi 0
    ldmfd sp!,{pc}^

.globl GetCpsr
GetCpsr:
    mrs r0, cpsr
    bx lr   
