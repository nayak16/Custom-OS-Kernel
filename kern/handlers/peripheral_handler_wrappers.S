.globl timer_handler

timer_handler:
    // Save Registers
    pusha
    // Call C handler
    call c_timer_handler
    // Restore Registers
    popa
    iret

.globl keyboard_handler

keyboard_handler:
    // Save Registers
    pusha
    // Call C handler
    call c_keyboard_handler
    // Restore Registers
    popa
    iret
