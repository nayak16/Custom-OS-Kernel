/* TODO: share wrapper macro between this file and exception handler */

.macro save_context
    pusha
    push %gs
    push %fs
    push %es
    push %ds
.endm
.macro restore_context_error_code
    pop %ds
    pop %es
    pop %fs
    pop %gs
    popa
    addl $4, %esp /* skip error_code */
    iret
.endm

.macro restore_context
    pop %ds
    pop %es
    pop %fs
    pop %gs
    popa
    iret
.endm

.macro wrapper func
    save_context
    pushl %esp
    call \func
    addl $4, %esp
    restore_context
.endm

.macro wrapper_error_code func
    save_context
    pushl %esp
    call \func
    addl $4, %esp
    restore_context_error_code
.endm

.globl page_fault_handler
page_fault_handler:
    wrapper_error_code page_fault_c_handler

.globl double_fault_handler
double_fault_handler:
    wrapper_error_code double_fault_c_handler

.globl division_error_handler
division_error_handler:
    wrapper division_error_c_handler

.globl debug_exception_handler
debug_exception_handler:
    wrapper debug_exception_c_handler

.globl breakpoint_handler
breakpoint_handler:
    wrapper breakpoint_c_handler

.globl overflow_handler
overflow_handler:
    wrapper overflow_c_handler

.globl bound_range_handler
bound_range_handler:
    wrapper bound_range_c_handler

.globl undef_op_handler
undef_op_handler:
    wrapper undef_op_c_handler

.globl no_math_handler
no_math_handler:
    wrapper no_math_c_handler

.globl coprocessor_segment_overrun_handler
coprocessor_segment_overrun_handler:
    wrapper coprocessor_segment_overrun_c_handler

.globl invalid_tss_handler
invalid_tss_handler:
    wrapper_error_code invalid_tss_c_handler

.globl segment_not_present_handler
segment_not_present_handler:
    wrapper_error_code segment_not_present_c_handler

.globl ss_fault_handler
ss_fault_handler:
    wrapper_error_code ss_fault_c_handler

.globl gp_fault_handler
gp_fault_handler:
    wrapper_error_code gp_fault_c_handler

.globl math_fault_handler
math_fault_handler:
    wrapper math_fault_c_handler

.globl align_fault_handler
align_fault_handler:
    wrapper_error_code align_fault_c_handler

.globl machine_check_fault_handler
machine_check_fault_handler:
    wrapper machine_check_fault_c_handler

.globl simd_fault_handler
simd_fault_handler:
    wrapper simd_fault_c_handler

