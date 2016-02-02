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
    /* Store system call parameters */
    stmfd sp!, {r0-r4, lr}

    /* Grab user cpsr and pc */
    mov r0, lr
    mrs r1, spsr

    /* Switch to system mode to get at the user's stack */
    msr cpsr_c, #0xDF

    /* Store the task's context */
    stmfd sp!, {r4-r12, lr}

    /* We don't actually need to store r0-r4, but the interrupt handler does */
    /* Make it look like we stored stuff, for compatibility */
    sub sp, sp, #16

    /* Store user cpsr and pc */
    stmfd sp!, {r0-r1}

    /* Store the user's sp as we will need to use it */
    /* This saves us from having to make another mode switch */
    mov r4, sp

    /* Switch back to supervisor mode */
    msr cpsr_c, #0xD3

    /* Get the current task */
    bl SchedulerGetCurrentTask

    /* We need the current task later, so store it for efficiency */
    mov r10, r0

    /* Move stack pointer to the correct location */
    mov r1, r4

    /* Update the current task's stack pointer */
    /* Current task is in r0 */
    /* New stack pointer is in r1 */
    bl TaskUpdateStackPointer

    /* Restore system call parameters */
    ldmfd sp!, {r0-r4, lr}

    /* Put the 5th system call parameter on the stack */
    /* This is where gcc expects to find it */
    stmfd sp!, {r4}

    /* Grab the swi instruciton */
    ldr r4, [lr, #-4]

    /* Isolate the system call number */
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

    /* Remove the 5th system call parameter from the stack */
    add sp, sp, #4

    /* Move current task and return value to correct locations */
    mov r1, r0 /* Return value is now in r1 */
    mov r0, r10 /* Current task is now in r0 */

    /* Store the system call result on the task's stack */
    bl TaskSetReturnValue

    /* Restore kernel state */
    /* This will jump back to whoever called KernelLeave() */
    ldmfd sp!, {r4-r12, pc}
