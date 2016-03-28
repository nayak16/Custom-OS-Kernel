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
    scheduler_num_ticks++;

    /* Wake up any sleeping threads */
    if (scheduler_wakeup(&sched) < 0){
        lprintf("Error waking up threads");
        MAGIC_BREAK;
    }

    /* Context switch into scheduler determined tcb,
     * possibly into a thread that was just woken up */
    uint32_t new_esp = context_switch(old_esp, -1);

    outb(INT_CTL_PORT, INT_ACK_CURRENT);
    return new_esp;
}

void c_keyboard_handler(){
    unsigned char aug_char = inb(KEYBOARD_PORT);
    outb(INT_CTL_PORT, INT_ACK_CURRENT);

    kh_type proc_char = process_scancode((uint32_t)aug_char);
    if (KH_HASDATA(proc_char) && KH_ISMAKE(proc_char)){
        if (keyboard_write(&keyboard, KH_GETCHAR(proc_char)) < 0)
            lprintf("keyboard buffer overflowed *beep*");
    }
}
