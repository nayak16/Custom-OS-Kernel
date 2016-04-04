.macro save_context
    subl $4, %esp /* Skip error_code */
    pushl $0 /* Store 0 as eax, in case of success */
    pushl %ecx
    pushl %edx
    pushl %ebx
    subl $4, %esp /* Skip save esp */
    pushl %ebp
    pushl %esi
    pushl %edi
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
    popl %edi
    popl %esi
    popl %ebp
    addl $4, %esp /* Skip restore esp */
    popl %ebx
    popl %edx
    popl %ecx
    addl $4, %esp /* Skip restore eax */
    addl $4, %esp /* Skip error_code */
    iret
.endm


/** @brief Wrapper for syscalls that take in no argument */
.macro no_arg_syscall_wrapper func
    save_context
    call \func
    restore_context
.endm

/** @brief Wrapper for syscalls that take in one argument */
.macro one_arg_syscall_wrapper func
    save_context
    pushl %esi /* push the argument onto the stack*/
    call \func /* call c handler with 1 argument */
    addl $4, %esp /* skip argument */
    restore_context
.endm

.macro two_arg_syscall_wrapper func
    save_context
    pushl 4(%esi) /* push 2nd argument onto the stack */
    pushl (%esi) /* push 1st argument onto the stack*/
    call \func /* call c handler with 1 argument */
    addl $8, %esp /* skip 2 arguments */
    restore_context
.endm

.macro four_arg_syscall_wrapper func
    save_context
    pushl 12(%esi) /* push 4th argument onto the stack */
    pushl 8(%esi) /* push 3rd argument onto the stack */
    pushl 4(%esi) /* push 2nd argument onto the stack */
    pushl (%esi) /* push 1st argument onto the stack*/
    call \func /* call c handler with 1 argument */
    addl $16, %esp /* skip 4 arguments */
    restore_context
.endm


.globl syscall_gettid_handler
syscall_gettid_handler:
    no_arg_syscall_wrapper syscall_gettid_c_handler

.globl syscall_fork_handler
syscall_fork_handler:
    save_context
    pushl %esp /* push stack pointer argument*/
    call syscall_fork_c_handler /* call c handler with 1 argument */
    addl $4, %esp /* skip argument */
    restore_context

.globl syscall_print_handler
syscall_print_handler:
    two_arg_syscall_wrapper syscall_print_c_handler

.globl syscall_readline_handler
syscall_readline_handler:
    two_arg_syscall_wrapper syscall_readline_c_handler

.globl syscall_exec_handler
syscall_exec_handler:
    two_arg_syscall_wrapper syscall_exec_c_handler

.globl syscall_vanish_handler
syscall_vanish_handler:
    no_arg_syscall_wrapper syscall_vanish_c_handler

.globl syscall_set_status_handler
syscall_set_status_handler:
    one_arg_syscall_wrapper syscall_set_status_c_handler

.globl syscall_halt_handler
syscall_halt_handler:
    save_context
    call syscall_halt_c_handler
    hlt /* if simics cmd fails to halt, halt here instead */
    restore_context

.globl syscall_new_pages_handler
syscall_new_pages_handler:
    two_arg_syscall_wrapper syscall_new_pages_c_handler

.globl syscall_remove_pages_handler
syscall_remove_pages_handler:
    one_arg_syscall_wrapper syscall_remove_pages_c_handler

.globl syscall_set_term_color_handler
syscall_set_term_color_handler:
    one_arg_syscall_wrapper syscall_set_term_color_c_handler

.globl syscall_set_cursor_pos_handler
syscall_set_cursor_pos_handler:
    two_arg_syscall_wrapper syscall_set_cursor_pos_c_handler

.globl syscall_get_cursor_pos_handler
syscall_get_cursor_pos_handler:
    two_arg_syscall_wrapper syscall_get_cursor_pos_c_handler

.globl syscall_yield_handler
syscall_yield_handler:
    save_context
    movl %esp, %eax /* Save esp */
    pushl %esi /* push the tid arg onto the stack*/
    pushl %eax /* Push old_esp arg onto the stack */
    call syscall_yield_c_handler /* call yield c handler */
    addl $8, %esp /* skip both arguments */
    restore_context

.globl syscall_deschedule_handler
syscall_deschedule_handler:
    save_context
    movl %esp, %eax /* Save esp */
    pushl %esi /* push the reject arg onto the stack*/
    pushl %eax /* Push old_esp arg onto the stack */
    call syscall_deschedule_c_handler /* call deschedule c handler */
    addl $8, %esp /* skip both arguments */
    restore_context

.globl syscall_make_runnable_handler
syscall_make_runnable_handler:
    one_arg_syscall_wrapper syscall_make_runnable_c_handler

.globl syscall_get_ticks_handler
syscall_get_ticks_handler:
    no_arg_syscall_wrapper syscall_get_ticks_c_handler

.globl syscall_sleep_handler
syscall_sleep_handler:
    save_context
    movl %esp, %eax /* Save esp */
    pushl %esi /* push the ticks arg onto the stack*/
    pushl %eax /* Push old_esp arg onto the stack */
    call syscall_sleep_c_handler /* call sleep c handler */
    addl $8, %esp /* skip both arguments */
    restore_context

.globl syscall_wait_handler
syscall_wait_handler:
    one_arg_syscall_wrapper syscall_wait_c_handler

.globl syscall_swexn_handler
syscall_swexn_handler:
    save_context
    pushl 12(%esi) /* push 4th argument onto the stack */
    pushl 8(%esi) /* push 3rd argument onto the stack */
    pushl 4(%esi) /* push 2nd argument onto the stack */
    pushl (%esi) /* push 1st argument onto the stack*/
    call syscall_swexn_c_handler /* call c handler with 1 argument */
    addl $16, %esp /* skip 4 arguments */
    restore_context



