/** @file peripheral_handlers.c
 *  @brief Implements handlers for peripheral hardware
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)
 *  @bug No known bugs
 */

#include <simics.h>
#include <x86/asm.h>
#include <x86/interrupt_defines.h>
#include <x86/timer_defines.h>
#include <smp/apic.h>
#include <smp/smp.h>
#include <stdio.h>
#include <scheduler.h>
#include <dispatcher.h>

/* access to buffer */
#include <kern_internals.h>

/* KEYBOARD_PORT define */
#include <x86/keyhelp.h>
#include <circ_buffer.h>

#define PIT_FREQ_CALIB_TICKS 10

uint32_t apic_calib_init_val = 0;
int pit_timer_ticks = 0;


/** @brief Implements the timer handler
 *  @param old_esp The stack pointer of the thread that was just running
 *  @return The stack pointer of the thread that was selected to run by the
 *  scheduler
 */
void c_pit_timer_handler(void) {
    pit_timer_ticks++;

    /* Check if specified time has passed */
    if (pit_timer_ticks == PIT_FREQ_CALIB_TICKS) {

        /* Calculate number of ticks elapsed */
        uint32_t init_lapic_val = lapic_read(LAPIC_TIMER_INIT);
        uint32_t cur_lapic_val = lapic_read(LAPIC_TIMER_CUR);
        uint32_t diff = init_lapic_val - cur_lapic_val;

        /* Set global calibrated value */
        apic_calib_init_val = diff / PIT_FREQ_CALIB_TICKS;

        /* Disable timer before sending new vals */
        lapic_write(LAPIC_TIMER_INIT, 0);

        /* Calibrate LAPIC Timer with calculated vals */
        lapic_write(LAPIC_TIMER_INIT, apic_calib_init_val);
        lapic_write(LAPIC_TIMER_DIV, LAPIC_X1);

        /* Turn off legacy PIT timer */
        outb(TIMER_MODE_IO_PORT, TIMER_ONE_SHOT);
    }

    /* Ack PIT timer */
    outb(INT_CTL_PORT, INT_ACK_CURRENT);
}

/** @brief Implements the timer handler
 *  @param old_esp The stack pointer of the thread that was just running
 *  @return The stack pointer of the thread that was selected to run by the
 *  scheduler
 */
uint32_t c_lapic_timer_handler(uint32_t old_esp) {

    //sched.num_ticks++;
    lprintf("CPU %d APIC timer!", smp_get_cpu());
    /* Wake up any sleeping threads */
    //scheduler_wakeup(&sched);

    /* Context switch into scheduler determined tcb,
     * possibly into a thread that was just woken up */
    //uint32_t new_esp = context_switch(old_esp, -1);

    /* Ack APIC timer interrupt */
    apic_eoi();
    return old_esp;
}


/** @brief Implements the keyboard handler
 *
 *  Reads a scancode from the keyboard port and processes it into a character
 *
 *  @return Void
 */
void c_keyboard_handler(){
    unsigned char aug_char = inb(KEYBOARD_PORT);
    outb(INT_CTL_PORT, INT_ACK_CURRENT);

    kh_type proc_char = process_scancode((uint32_t)aug_char);
    if (KH_HASDATA(proc_char) && KH_ISMAKE(proc_char)){
        if (keyboard_write(&keyboard, KH_GETCHAR(proc_char)) < 0)
            lprintf("keyboard buffer overflowed *beep*");
    }
}
