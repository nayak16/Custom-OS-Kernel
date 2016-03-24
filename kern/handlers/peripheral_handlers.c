#include <simics.h>
#include <x86/asm.h>
#include <x86/interrupt_defines.h>
#include <stdio.h>
#include <scheduler.h>
#include <dispatcher.h>

/* access to buffer */
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
    unsigned char aug_char = inb(KEYBOARD_PORT);
    outb(INT_CTL_PORT, INT_ACK_CURRENT);
    /* 0 extend aug_char and save into keyboard_buffer */
    if (circ_buf_write(&keyboard_buffer, (void *)((uint32_t)aug_char)) < 0)
        lprintf("keyboard buffer overflowed *beep*");
    return;
}
