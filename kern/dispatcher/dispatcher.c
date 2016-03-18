/** @file
 *
 */

#include <pcb.h>
#include <tcb.h>
#include <x86/cr.h>
#include <dispatcher.h>
#include <kern_internals.h>

#include <simics.h>

void initial_mode_switch(tcb_t *tcb) {

}


uint32_t context_switch(uint32_t old_esp, int target_tid) {

    /* Switch to scheduler selected tid */
    uint32_t new_esp;
    if (target_tid < 0) {

        /* Save old TCB */
        if (scheduler_save_running_tcb(&sched, old_esp) < 0) {
            lprintf("Current tid tcb does not exist !!!");
            MAGIC_BREAK;
        }
        /* Get next available TCB */
        tcb_t *next_tcb;
        if (scheduler_get_next_tcb(&sched, &next_tcb) < 0) {
            // TODO: PANIC
            lprintf("You just got shrekt");
            MAGIC_BREAK;
        }

        /* Set current running tcb and get new esp */
        if (scheduler_set_running_tcb(&sched, next_tcb, &new_esp) < 0) {
            lprintf("Couldnt set running tcb!!");
            // TODO: PANIC
            MAGIC_BREAK;
        }
    } else {
        /* Switch to target_tid tcb (used in yield) */
    }
    return new_esp;

}

