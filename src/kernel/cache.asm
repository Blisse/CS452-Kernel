.globl CacheInit
CacheInit:
    /* Invalidate instruction and data cache */
    mov r0, #0
    mcr p15, 0, r0, c7, c7, 0

    /* Enable instruction and data cache */
    mrc p15, 0, r0, c1, c0, 0
    orr r0, r0, #0x1000
    orr r0, r0, #0x4
    mcr p15, 0, r0, c1, c0, 0

    /* Return */
    bx lr
