.globl TrapInstallHandler
TrapInstallHandler:
    /* Load the address of the software interrupt controller */
    mov r0, #0x00000028

    /* Load the address of the software interrupt handler */
    ldr r1, =TrapEntry

    /* Install interrupt handler to interrupt controller */
    str r1, [r0] 
    bx lr

@ This function is passed 2 parameters
@ R0 contains the user's stack
.globl TrapReturn
TrapReturn:
    /* Store kernel state */
    stmfd sp!, {r4-r12, lr}
    
    /* Switch to system mode */
    msr cpsr_c, #0xDF

    /* Restore user's stack pointer */
    mov sp, r0

    /* Restore the user state */
    ldmfd sp!, {r0, r2-r12, lr}

    /* Switch back to supervisor mode */
    msr cpsr_c, #0xD3

    /* Jump back to user mode */
    msr spsr, r2
    movs pc, r3

.globl TrapEntry
TrapEntry:
    /* Store registers we are about to clobber */
    /* If there is a 5th system call parameter, it will be in R4 */
    /* This will push R4 on to the stack, which is where GCC expects */
    /* the 5th parameter to be anyway */
    stmfd sp!, {r4-r6, lr}

    /* Grab the swi instruction */
    ldr r4, [lr, #-4]

    /* Isolate the system call number */
    /* Take the complement of the mask, then perform an "AND" */
    bic r4, r4, #0xFF000000

    /* Convert system call number to table offset */
    mov r5, #4
    mul r6, r5, r4

    /* Grab the system call table */
    ldr r4, =g_systemCallTable

    /* Grab the system call from the system call table */
    ldr r4, [r4, r6]

    /* Make the system call */
    /* Return value will be in r0 */
    /* NOTE: Arm v4 doesn't support blx so we have to emulate it.
             The mov from pc to lr takes in to consideration the 
             processor pipeline. */
    mov lr, pc
    mov pc, r4

    /* Restore clobbered registers */
    /* Put the LR in to R3 early as an optimization */
    ldmfd sp!, {r4-r6, lr}

    /* User CPSR is in SPSR */
    mrs r2, spsr

    /* User PC is in LR */
    mov r3, lr

    /* Switch to system mode */
    msr cpsr_c, #0xDF

    /* Store user registers */
    stmfd sp!, {r0, r2-r12, lr}

    /* Switch back to supervisor mode */
    msr cpsr_c, #0xD3

    /* Restore kernel state */
    /* This will jump back to whoever called TrapReturn() */
    ldmfd sp!, {r4-r12, pc}
