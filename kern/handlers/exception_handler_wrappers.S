.globl page_fault_handler
page_fault_handler:
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
    call page_fault_c_handler // call our keyboard_c_handler
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
    iret                    // return from handler & restore EFLAG registers

.globl double_fault_handler
double_fault_handler:
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
    call double_fault_c_handler // call our keyboard_c_handler
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
