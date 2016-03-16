
#include <simics.h>
#include <x86/asm.h>
#include <x86/interrupt_defines.h>
#include <stdio.h>
#include <scheduler.h>
#include <dispatcher.h>
#include <kern_internals.h>
/* KEYBOARD_PORT define */
#include <x86/keyhelp.h>

int key_buffer_read(){
    // temporary
    return 0;
}

int readchar(void){
    int aug_char;
    kh_type proc_char;
    /* no keys to be read */
    aug_char = key_buffer_read();
    if (aug_char == -1) return -1;
    /* bit extend with 0's before returning */
    proc_char = process_scancode(aug_char);
    if (KH_HASDATA(proc_char) && KH_ISMAKE(proc_char)){
        return KH_GETCHAR(proc_char);
    }
    /* key stroke is a non-data or non-keystroke-down key press */
    return -1;
}

void c_timer_handler(void) {

    // TODO: Save contextR(proc_char)
    // Get next tcb
    // TODO: Restore context or enter user mode for first time
    outb(INT_CTL_PORT, INT_ACK_CURRENT);
    return;
}

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
}
