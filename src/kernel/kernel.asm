@ This function is passed 2 parameters
@ User's PC is passed on the kernel stack
@ SPSR is passed on the kernel stack
.globl KernelSaveUserContext
KernelSaveUserContext:
    /* Store registers we are about to clobber */
    stmfd sp!, {r0, r1}

    /* Load the user's PC and SPSR from the kernel stack */
    ldr r0, [sp, #8]
    ldr r1, [sp, #12]

    /* Switch to system mode */
    msr cpsr_c, #0xDF

    /* Store user's PC and CPSR */
    stmfd sp!, {r0, r1}
    
    /* Switch back to supervisor mode */
    msr cpsr_c, #0xD3

    /* Restore clobbered registers */
    ldmfd sp!, {r0, r1}

    /* Switch to system mode */
    msr cpsr_c, #0xDF

    /* Store user registers */
    stmfd sp!, {r0-r12, lr}
    
    /* Save pointer to user's stack */
    mov r4, sp

    /* Switch back to supervisor mode */
    msr cpsr_c, #0xD3

    /* Get the current task */
    stmfd sp!, {lr}
    bl SchedulerGetCurrentTask
    ldmfd sp!, {lr}

    /* Update the current task's stack pointer */
    /* The current task is in R0 */
    /* The new stack pointer is in R1 */
    mov r1, r4
    b TaskUpdateStackPointer

@ This function is passed 1 parameter
@ R0 contains the user's stack
.globl KernelLeave
KernelLeave:
    /* Store kernel state */
    stmfd sp!, {r4-r12, lr}

    /* Switch to system mode */
    msr cpsr_c, #0xDF

    /* Restore user's stack pointer */
    mov sp, r0

    /* Restore the user state */
    ldmfd sp!, {r0-r12, lr}    

    /* Switch back to supervisor mode */
    msr cpsr_c, #0xD3

    /* Store registers we are about to clobber */
    stmfd sp!, {r0, r1}

    /* Switch to system mode */
    msr cpsr_c, #0xDF

    /* Restore user's PC and CPSR */
    ldmfd sp!, {r0, r1}

    /* Switch back to supervisor mode */
    msr cpsr_c, #0xD3

    /* Store user PC and CPSR back to their original locations */
    mov lr, r0
    msr spsr, r1

    /* Restore clobbered registers */
    ldmfd sp!, {r0, r1}

    /* Jump back to user mode */
    movs pc, lr

.globl KernelEnter
KernelEnter:
    /* Restore kernel state */
    /* This will jump back to whoever called KernelLeave() */
    ldmfd sp!, {r4-r12, pc}
