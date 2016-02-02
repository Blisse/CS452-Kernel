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
    stmfd sp!, {r0, r1}

    /* Caclulate address of user's pc */
    sub r0, lr, #4

    /* Save the CPSR */
    mrs r1, spsr

    /* Switch to supervisor mode */
    msr cpsr_c, #0xD3

    /* KernelSaveUserContext expects the user's pc and cpsr on the stack */
    stmfd sp!, {r0, r1}

    /* Switch back to irq mode */
    msr cpsr_c, #0xD2

    /* Restore clobbered registers */
    ldmfd sp!, {r0, r1}

    /* Switch to supervisor mode since the irq stack is small */
    msr cpsr_c, #0xD3

    /* Save user state */
    /* r0-r12 can now be used without needing to save */
    bl KernelSaveUserContext

    /* Pop the user pc and cpsr off the stack */
    add sp, sp, #8

    /* Call the interrupt handler */
    bl InterruptHandler

    /* Enter the kernel */
    b KernelEnter
