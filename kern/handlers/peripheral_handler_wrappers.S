

.globl keyboard_handler
keyboard_handler:
    // Save Registers
    pusha
    // Call C handler
    call c_keyboard_handler
    // Restore Registers
    popa
    iret

.globl timer_handler
timer_handler:
    // Save Register Context
    pusha
    push %ds
    push %es
    push %fs
    push %gs
    // Pass args
    push %esp
    // Call C handler
    call c_timer_handler
    movl %eax, %esp
    // Restore Registers
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa
    iret
