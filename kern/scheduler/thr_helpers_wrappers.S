


.macro build_iret_stack
    movl %esp, %eax /* Save current esp in eax */
    push %ss        /* Save ss */
    addl $8, %eax   /* Account for rtn address and arg */
    pushl %eax      /* Save esp */
    pushf           /* Save EFLAGS */
    push %cs        /* Save cs */
    subl $8, %eax   /* eax now points to rtn address */
    pushl (%eax)    /* Save rtn address */
.endm


.macro save_regs
    subl $4, %esp /* Skip error code */
    pushl $0   /* Store 0 in eax, in case of success */
    pushl %ecx
    pushl %edx
    pushl %ebx
    subl $4, %esp   /* Skip esp save */
    pushl %ebp
    pushl %esi
    pushl %edi
    push %gs
    push %fs
    push %es
    push %ds
.endm

.macro restore_regs
    pop %ds
    pop %es
    pop %fs
    pop %gs
    popl %edi
    popl %esi
    popl %ebp
    addl $4, %esp
    popl %ebx
    popl %edx
    popl %ecx
    addl $4, %esp /* skip eax */
    addl $4, %esp /* skip error code */
.endm

.globl thr_kern_deschedule
thr_kern_deschedule:
    /* Construct iret stack so we can context switch back */
    build_iret_stack
    /* Save GP registers */
    save_regs

    pushl 4(%eax)   /* Push reject arg onto stack */
    movl %esp, %eax
    addl $4, %eax   /* eax now is esp before arg push */
    pushl %eax      /* Push old_esp arg onto stack */
    call thr_deschedule
    /* This means that deschedule returned with an error
     * eax contains negative error code */
    addl $8, %esp   /* Skip two arguments */
    /* Restore GP registers */
    restore_regs
    /* Skip eax restore and iret stack arguments */
    addl $24, %esp
    ret

.globl thr_kern_yield
thr_kern_yield:
    /* Construct iret stack so we can context switch back */
    build_iret_stack
    /* Save GP registers */
    save_regs

    pushl 4(%eax)   /* Push tid arg onto stack */
    movl %esp, %eax
    addl $4, %eax   /* eax now is esp before arg push */
    pushl %eax      /* Push old_esp arg onto stack */
    call thr_yield
    /* This means that yield returned with an error
     * eax contains negative error code */
    addl $8, %esp   /* Skip two arguments */
    /* Restore GP registers */
    restore_regs
    /* Skip eax restore and iret stack arguments */
    addl $24, %esp
    ret


