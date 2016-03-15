
#include <simics.h>
#include <x86/asm.h>
#include <x86/interrupt_defines.h>
#include <stdio.h>
#include <scheduler.h>
#include <kern_internals.h>


void c_timer_handler(void) {

    // Save context
    outb(INT_CTL_PORT, INT_ACK_CURRENT);
    return;
}

void c_keyboard_handler(void) {
    lprintf("Key Pressed!");
    outb(INT_CTL_PORT, INT_ACK_CURRENT);
}
