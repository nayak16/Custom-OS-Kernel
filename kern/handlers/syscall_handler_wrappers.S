.macro save_context
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
.endm

.macro restore_context
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



.globl syscall_gettid_handler
syscall_gettid_handler:
    no_arg_syscall_wrapper syscall_gettid_c_handler

.globl syscall_fork_handler
syscall_fork_handler:
    no_arg_syscall_wrapper syscall_fork_c_handler

.globl syscall_print_handler
syscall_print_handler:
    two_arg_syscall_wrapper syscall_print_c_handler

