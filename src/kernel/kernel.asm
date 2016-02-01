@ This function is passed 1 parameter
@ R0 contains the user's PC
.globl KernelSaveUserContext
KernelSaveUserContext:
    /* User CPSR is in SPSR */
    mrs r3, spsr

    /* Switch to system mode */
    msr cpsr_c, #0xDF

    /* Store user registers */
    stmfd sp!, {r0, r3-r12, lr}

    /* Leave room for a return value (if one exists) */
    sub sp, sp, #4

    /* Save pointer to user's stack */
    mov r1, sp

    /* Switch back to supervisor mode */
    msr cpsr_c, #0xD3

    /* Get the current task */
    stmfd sp!, {lr}
    bl SchedulerGetCurrentTask
    ldmfd sp!, {lr}

    /* Update the current task's stack pointer */
    /* The current task is in R0 */
    /* The new stack pointer is in R1 */
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
    ldmfd sp!, {r0, r2-r12, lr}

    /* Switch back to supervisor mode */
    msr cpsr_c, #0xD3

    /* Jump back to user mode */
    msr spsr, r3
    movs pc, r2

.globl KernelEnter
KernelEnter:
    /* Restore kernel state */
    /* This will jump back to whoever called KernelLeave() */
    ldmfd sp!, {r4-r12, pc}
