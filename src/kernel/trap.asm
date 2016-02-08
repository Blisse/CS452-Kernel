.globl TrapInstallHandler
TrapInstallHandler:
    /* Load the address of the software interrupt controller */
    mov r0, #0x00000028

    /* Load the address of the software interrupt handler */
    ldr r1, =TrapEnter

    /* Install interrupt handler to interrupt controller */
    str r1, [r0] 
    bx lr

.globl TrapEnter
TrapEnter:
    /* DO NOT CLOBBER R0-R4 */
    /* THEY HAVE THE SYSTEM CALL PARAMETERS */

    /* Switch to system mode to get at the user's stack */
    msr cpsr_c, #0xDF

    /* Store the task's context */
    stmfd sp!, {r4-r12, lr}

    /* We don't actually need to store r0-r4, but the interrupt handler does */
    /* Make it look like we stored stuff, for compatibility */
    sub sp, sp, #16    

    /* Store the user's sp as we will need to use it */
    /* This saves us from having to make another mode switch */
    mov r10, sp

    /* Switch back to supervisor mode */
    msr cpsr_c, #0xD3

    /* Grab user cpsr and pc */
    mov r5, lr
    mrs r6, spsr

    /* Store user cpsr and pc */
    stmfd r10!, {r5-r6}

    /* Get the current task */
    ldr r5, =g_currentTd
    ldr r5, [r5]

    /* Update the current task's stack pointer */
    /* Current task is in r5 */
    /* Task stack pointer is in r10 */
    str r10, [r5]

    /* Put the 5th system call parameter on the stack */
    /* This is where gcc expects to find it */
    sub sp, sp, #4
    str r4, [sp]

    /* Grab the swi instruciton */
    ldr r5, [lr, #-4]

    /* Isolate the system call number */
    bic r5, r5, #0xFF000000

    /* Convert system call number to table offset */
    mov r6, #4
    mul r7, r6, r5

    /* Grab the system call table */
    ldr r5, =g_systemCallTable

    /* Grab the system call from the system call table */
    ldr r5, [r5, r7]

    /* Make the system call */
    /* Return value will be in r0 */
    /* NOTE: Arm v4 doesn't support blx so we have to emulate it.
             The mov from pc to lr takes in to consideration the 
             processor pipeline. */
    mov lr, pc
    mov pc, r5

    /* Remove the 5th system call parameter from the stack */
    add sp, sp, #4

    /* Store the system call result on the task's stack */
    /* Current task is in r0 */
    /* Task stack pointer is in r10 */
    /* Task's r0 is 8 bytes ahead of its stack pointer */
    str r0, [r10, #8]

    /* Restore kernel state */
    /* This will jump back to whoever called KernelLeave() */
    ldmfd sp!, {r4-r12, pc}
