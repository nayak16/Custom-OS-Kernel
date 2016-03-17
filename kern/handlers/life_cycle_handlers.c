/** @file life_cycle_handlers.c
 *  @brief implements life cycle syscall handlers
 *
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)
 *  @bug No known bugs.
 */

#include <kern_internals.h>

int syscall_fork_c_handler(uint32_t *saved_regs){


    /* Copy current pcb */
    int new_tid = scheduler_copy_current_pcb(&sched, saved_regs);

    // TODO: Flush TLB mappings using INVLPG

    return new_tid;
}

