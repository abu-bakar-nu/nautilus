.global test_paging
test_paging:

mov %cr3, %eax
// andl $0xfffffffffffffffe, %eax
andl $0, %eax
mov %eax, %cr3

// mov 10, %edx
// mov (%edx), %eax

ret

