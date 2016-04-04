/** @file exception_handlers.c
 *  @brief implements various exception handlers
 *
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)
 *  @bug No known bugs.
 */

#include <x86/cr.h>

#include <simics.h>
/* panic inc */
#include <stdlib.h>
#include <debug.h>
#include <thr_helpers.h>

#include <x86/cr.h>
#include <kern_internals.h>

#include <tcb.h>
#include <ureg.h>

int ureg_from_stack(ureg_t *ureg, uint32_t cause, uint32_t *stack){
    return 0;
}

int swexn_execute(uint32_t cause, uint32_t *stack){
    /* get the current context */
    ureg_t ureg;
    if (ureg_from_stack(&ureg, cause, stack) < 0) return -1;
    return -1;
    /* */

}

void page_fault_c_handler(uint32_t *stack){
    lprintf("Segmentation Fault");
    print_control_regs();
    MAGIC_BREAK;
    if (swexn_execute(SWEXN_CAUSE_PAGEFAULT, stack) == 0){
        return;
    }
    thr_set_status(-2);
    thr_vanish();
}

void double_fault_c_handler(void){
    print_control_regs();
    lprintf("Double fault occured!");
    MAGIC_BREAK;
}
