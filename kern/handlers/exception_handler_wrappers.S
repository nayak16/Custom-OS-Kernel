/* TODO: share wrapper macro between this file and exception handler */

.macro save_context
    pusha
    push %gs
    push %fs
    push %es
    push %ds
.endm
.macro restore_context
    pop %ds
    pop %es
    pop %fs
    pop %gs
    popa
    addl $4, %esp /* skip error_code */
    iret
.endm

.macro wrapper func
    save_context
    pushl %esp
    call \func
    addl $4, %esp
    restore_context
.endm

.globl page_fault_handler
page_fault_handler:
    wrapper page_fault_c_handler

.globl double_fault_handler
double_fault_handler:
    wrapper double_fault_c_handler
