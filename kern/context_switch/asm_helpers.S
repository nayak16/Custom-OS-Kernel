
#include <simics.h>

.globl restore_context_asm

/* Struct tcb_t passed in */
restore_context_asm:
    /* Save vars on stack */

    movl 4(%esp), %eax
    movl 8(%esp), %ebx
    movl 12(%esp), %ecx
    movl 16(%esp), %edx
    movl 20(%esp), %esi
    movl 24(%esp), %edi
    movl 28(%esp), %ebp
    // new esp: 32(%esp), %esp
    // eflags: 36(%esp), eflags
    push 36(%esp) // Push ss
    push 52(%esp) // Push esp
    push 40(%esp)         // Push eflags
    push 52(%esp) // Push cs
    push 60(%esp) // Push eip
    iret
