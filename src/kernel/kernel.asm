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
