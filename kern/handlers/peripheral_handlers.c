
#include <simics.h>
#include <x86/asm.h>
#include <x86/interrupt_defines.h>
#include <stdio.h>
#include <scheduler.h>
#include <dispatcher.h>
#include <kern_internals.h>
/* KEYBOARD_PORT define */
#include <x86/keyhelp.h>

void c_timer_handler(void) {

    // TODO: Save context
    // Get next tcb
    // TODO: Restore context or enter user mode for first time
    outb(INT_CTL_PORT, INT_ACK_CURRENT);
    return;
}

uint32_t c_keyboard_handler(uint32_t old_esp) {
    lprintf("Key Pressed!");
    MAGIC_BREAK;
    uint32_t new_esp = context_switch(old_esp, -1);
    /*char c = */inb(KEYBOARD_PORT);
    outb(INT_CTL_PORT, INT_ACK_CURRENT);
    return new_esp;
}
