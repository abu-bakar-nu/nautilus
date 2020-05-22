.global paging_on
paging_on:

    movl $1024, %ecx
    movl $pd, %edx
    movl $0x83, %eax                    // entry is present, rw, pagesize = 4MB
    andl $0xffdfffff, %eax               // set bit 21 to zero

.write_pde:
    movl %eax, (%edx)
    addl $0x400000, %eax
    addl $0x4, %edx
    loop .write_pde

    /* put pd address in cr3 */
    movl $pd, %eax
    movl %eax, %cr3

    /* put pse bit in cr4 */
    movl %cr4, %eax
    orl $(1<<4), %eax
    movl %eax, %cr4

    /* paging enable */
    movl %cr0, %eax
    orl $(1<<31), %eax

    /* make sure we're in "normal cache mode" */
    movl $~(3<<29), %ebx
    andl %ebx, %eax

    movl %eax, %cr0

    ret