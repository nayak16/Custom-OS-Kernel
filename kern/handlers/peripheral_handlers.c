#include <simics.h>
#include <x86/asm.h>
#include <x86/interrupt_defines.h>
#include <stdio.h>
#include <scheduler.h>
#include <dispatcher.h>
#include <kern_internals.h>
/* KEYBOARD_PORT define */
#include <x86/keyhelp.h>
#include <circ_buffer.h>


uint32_t c_timer_handler(uint32_t old_esp) {
    uint32_t new_esp = context_switch(old_esp, -1);
    outb(INT_CTL_PORT, INT_ACK_CURRENT);
    return new_esp;
}

void c_keyboard_handler(){
    inb(KEYBOARD_PORT);

    outb(INT_CTL_PORT, INT_ACK_CURRENT);
    return;
}
/* manual keyboard context switching code
uint32_t c_keyboard_handler(uint32_t old_esp) {
    char aug_char = inb(KEYBOARD_PORT);
    outb(INT_CTL_PORT, INT_ACK_CURRENT);
    kh_type proc_char = process_scancode(aug_char);
    if (KH_HASDATA(proc_char) && KH_ISMAKE(proc_char)){
        lprintf("Character read: %c", KH_GETCHAR(proc_char));
        uint32_t new_esp = context_switch(old_esp, -1);
        return new_esp;
    }
    return old_esp;
} */
