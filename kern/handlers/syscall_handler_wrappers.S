/* TODO: share wrapper macro between this file and exception handler */
.macro wrapper func
    pushl %ecx
    pushl %edx
    pushl %ebx
    pushl %ebp
    pushl %esi
    pushl %edi
    pushl %ss
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs
    call \func
    popl %gs
    popl %fs
    popl %es
    popl %ds
    popl %ss
    popl %edi
    popl %esi
    popl %ebp
    popl %ebx
    popl %edx
    popl %ecx
    iret
.endm



.globl syscall_gettid_handler
syscall_gettid_handler:
    wrapper syscall_gettid_c_handler

.globl syscall_fork_handler
syscall_fork_handler:
    wrapper syscall_fork_c_handler

