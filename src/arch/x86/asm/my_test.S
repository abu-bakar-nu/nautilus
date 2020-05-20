.global test_paging
test_paging:

mov %cr3, %eax
// add $4, %eax
andl $0xfffffffffffffffe, 4(%eax)
mov (0x799999), %eax

ret

