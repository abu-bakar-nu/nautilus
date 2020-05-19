.global paging_on
paging_on:

    movl $1024, %ecx
    movl $pd, %edx
    movl $0x83, %eax                    // entry is present, rw, pagesize = 4MB
    andl $0xffffffffffdfffff, %eax      // set bit 21 to zero

.write_pde:
    movl %eax, (%edx)
    addl $0x400000, %eax
    addl $0x4, %edx
    loop .write_pde

    /* put pd address in cr3 */
    movl $pd, %eax
    mov %eax, %cr3

    /* put pse bit in cr4 */
    mov %cr4, %eax
    or $(1<<4), %eax
    mov %eax, %cr4

    /* paging enable */
    mov %cr0, %eax
    orl $(1<<31), %eax

    /* make sure we're in "normal cache mode" */
    movl $~(3<<29), %ebx
    andl %ebx, %eax

    mov %eax, %cr0

    ret