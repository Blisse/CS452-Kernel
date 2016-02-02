.globl InterruptInstallHandler
InterruptInstallHandler:
    /* Load the address of the software interrupt controller */
    mov r0, #0x00000038

    /* Load the address of the software interrupt handler */
    ldr r1, =InterruptEnter

    /* Install interrupt handler to interrupt controller */
    str r1, [r0]

    /* Switch to irq mode */
    msr cpsr_c, #0xD2

    /* Setup interrupt stack */
    ldr sp, =0x003FFFFC

    /* Switch back to supervisor mode */
    msr cpsr_c, #0xD3

    /* Return */
    bx lr

.globl InterruptEnter
InterruptEnter:
    /* Store registers we need to clobber */
    stmfd sp!, {r0-r1}

    /* Grab user cpsr and pc */
    /* Address of user pc must be calculated*/
    sub r0, lr, #4
    mrs r1, spsr

    /* Switch to system mode to get at the user's stack */
    msr cpsr_c, #0xDF

    /* Store user cpsr and pc */
    /* These are stored first so that we have easy access to the */
    /* task's r0.  r0 is important since it's the task's return value */
    stmfd sp!, {r0-r1}

    /* Switch back to irq mode */
    msr cpsr_c, #0xD2

    /* Restore clobbered registers */
    ldmfd sp!, {r0-r1}

    /* Switch to system mode to get at the user's stack */
    msr cpsr_c, #0xDF

    /* Store task context */
    stmfd sp!, {r0-r12, lr}

    /* Store the user's sp as we will need to use it */
    /* This saves us from having to make another mode switch */
    mov r4, sp

    /* Switch to supervisor mode */
    msr cpsr_c, #0xD3

    /* Get the current task */
    bl SchedulerGetCurrentTask

    /* Move stack pointer to the correct location */
    mov r1, r4

    /* Update the current task's stack pointer */
    /* Current task is in r0 */
    /* New stack pointer is in r1 */
    bl TaskUpdateStackPointer

    /* Call the interrupt handler */
    bl InterruptHandler

    /* Restore kernel state */
    /* This will jump back to whoever called KernelLeave() */
    ldmfd sp!, {r4-r12, pc}
