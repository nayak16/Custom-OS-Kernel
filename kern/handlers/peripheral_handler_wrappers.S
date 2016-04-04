

.globl keyboard_handler
keyboard_handler:
    // Save Registers
    subl $4, %esp /* skip error code */
    pusha
    push %gs
    push %fs
    push %es
    push %ds
    // Call C handler
    call c_keyboard_handler
    // Restore Registers
    pop %ds
    pop %es
    pop %fs
    pop %gs
    popa
    addl $4, %esp /* skip error code */
    iret

.globl timer_handler
timer_handler:
    // Save Register Context
    subl $4, %esp /* skip error code */
    pusha
    push %gs
    push %fs
    push %es
    push %ds
    // Pass args
    push %esp
    // Call C handler
    call c_timer_handler
    push %eax
    call restore_context
    /* SHOULD NEVER RETURN */
