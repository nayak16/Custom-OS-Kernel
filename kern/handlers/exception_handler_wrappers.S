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

.globl page_fault_handler
page_fault_handler:
    wrapper page_fault_c_handler

.globl double_fault_handler
double_fault_handler:
    wrapper double_fault_c_handler
