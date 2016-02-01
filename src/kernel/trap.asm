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
    /* Save the function parameters on stack */
    stmfd sp!, {r0-r3}

    /* Save the user's pc */
    stmfd sp!, {lr}

    /* Find the current task */
    bl SchedulerGetCurrentTask

    /* Retrieve the user's pc from the stack */
    ldmfd sp, {r1}

    /* Save user state */
    /* r4-r12 can now be used without needing to save */
    bl KernelSaveUserContext

    /* Pop the user's pc off the stack */
    ldmfd sp!, {lr}

    /* Pop the function parameters off the stack */
    ldmfd sp!, {r0-r3}

    /* Put the 5th system call parameter on the stack */
    /* This is where gcc is expecting to find it */
    stmfd sp!, {r4}

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

    /* Remove 5th system call parameter from stack */
    add sp, sp, #4

    /* Move the system call result so it doesn't get clobbered */
    mov r4, r0

    /* Find the current task */
    bl SchedulerGetCurrentTask

    /* Setup the function call parameters */
    mov r1, r4

    /* Store the system call result on the user's stack */
    /* The current stack is in r0 */
    /* The system call return value is in r1 */
    bl TaskSetReturnValue

    /* Enter the kernel */
    b KernelEnter
