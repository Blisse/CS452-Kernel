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

    /* Pop the user's pc and cpsr off the stack */
    ldmfd sp!, {r0-r1}

    /* Switch back to supervisor mode */
    msr cpsr_c, #0xD3

    /* Restore user's pc and cpsr */
    mov lr, r0
    msr spsr, r1

    /* Switch to system mode */
    msr cpsr_c, #0xDF

    /* Restore the task's context */
    ldmfd sp!, {r0-r12, lr}

    /* Switch back to supervisor mode */
    msr cpsr_c, #0xD3

    /* Jump back to user mode */
    movs pc, lr
