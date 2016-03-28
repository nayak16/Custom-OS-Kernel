
.globl thr_kern_deschedule

thr_kern_deschedule:
    movl %esp, %eax /* Save current esp in eax */
    push %ss        /* Save ss */
    addl $8, %eax   /* Account for rtn address and arg */
    pushl %eax      /* Save esp */
    pushf           /* Save EFLAGS */
    push %cs        /* Save cs */
    subl $8, %eax   /* eax now points to rtn address */
    pushl (%eax)    /* Save rtn address */

    /* Save GP registers */
    pushl $0   /* Store 0 in eax, in case of success */
    pushl %ecx
    pushl %edx
    pushl %ebx
    subl $4, %esp   /* Skip esp save */
    pushl %ebp
    pushl %esi
    pushl %edi
    push %ds
    push %es
    push %fs
    push %gs
    pushl 4(%eax)   /* Push reject arg onto stack */
    movl %esp, %eax
    addl $4, %eax   /* eax now is esp before arg push */
    pushl %eax      /* Push old_esp arg onto stack */
    call thr_deschedule
    /* This means that deschedule returned with an error
     * eax contains negative error code */
    addl $8, %esp   /* Skip two arguments */
    /* Restore GP registers */
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popl %edi
    popl %esi
    popl %ebp
    addl $4, %esp
    popl %ebx
    popl %edx
    popl %ecx
    /* Skip eax restore and iret stack arguments */
    addl $24, %esp
    ret


