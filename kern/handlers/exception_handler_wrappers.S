/* TODO: share wrapper macro between this file and exception handler */

.macro save_context
    pusha
    push %ds
    push %es
    push %fs
    push %gs
.endm
.macro restore_context
    pop %gs
    pop %fs
    pop %es
    pop %ds
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
