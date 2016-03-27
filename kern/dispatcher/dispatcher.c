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

    /* Defer current TCB */
    if (scheduler_defer_current_tcb(&sched, old_esp) < 0) {
        lprintf("Current tid tcb does not exist !!!");
        MAGIC_BREAK;
    }
    /* Wake up any sleeping threads */
    if (scheduler_wakeup(&sched) < 0){
        lprintf("Error waking up threads");
        MAGIC_BREAK;
    }

    tcb_t *next_tcb;
    /* Switch to scheduler selected tid */
    if (target_tid < 0) {

        /* Get next available TCB */
        if (scheduler_get_next_tcb(&sched, &next_tcb) < 0) {
            // TODO: PANIC
            lprintf("You just got shrekt");
            MAGIC_BREAK;
        }

    } else {

    }
    uint32_t new_esp;
    /* Set current running tcb and get new esp */
    if (scheduler_set_running_tcb(&sched, next_tcb, &new_esp) < 0) {
        lprintf("Couldnt set running tcb!!");
        // TODO: PANIC
        MAGIC_BREAK;
    }

    return new_esp;

}

