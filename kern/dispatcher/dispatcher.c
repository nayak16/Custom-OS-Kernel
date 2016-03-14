/** @file
 *
 */

#include <pcb.h>
#include <tcb.h>
#include <x86/cr.h>
#include <dispatcher.h>

#include <simics.h>

void initial_mode_switch(tcb_t *tcb) {

    /* Set next kernel stack pointer */
    set_esp0((uint32_t) tcb->k_stack);

    initial_mode_switch_asm(*tcb);
}


