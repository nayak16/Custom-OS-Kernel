/** @file
 *
 */

#include <pcb.h>
#include <tcb.h>
#include <x86/cr.h>
#include <context_switch.h>

#include <simics.h>

void restore_context(tcb_t *tcb) {

    /* Set next kernel stack pointer */
    set_esp0((uint32_t) tcb->k_stack);
    restore_context_asm(*tcb);
}


