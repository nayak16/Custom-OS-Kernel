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
#include <string.h>

#include <x86/cr.h>
#include <kern_internals.h>

#include <special_reg_cntrl.h>

#include <tcb.h>
#include <ureg.h>
#include <x86/seg.h>

int ureg_from_stack(ureg_t *ureg, uint32_t cause, uint32_t *stack){
    if (stack == NULL) return -1;
    ureg->cause = cause;
    ureg->cr2 = get_cr2();
    memcpy(&(ureg->ds), stack, REGS_SIZE * sizeof(unsigned int));
    return 0;
}

int swexn_execute(uint32_t cause, uint32_t *stack){
    /* get the current context */
    ureg_t ureg;
    if (ureg_from_stack(&ureg, cause, stack) < 0) return -1;
    /* get the current tcb */
    tcb_t *cur_tcb;
    if (scheduler_get_current_tcb(&sched, &cur_tcb) < 0) return -2;

    /* deregister current handler and save info about handler if
     * present */
    void *esp3 = NULL;
    void (*eip)(void *arg, ureg_t *ureg) = NULL;
    void *arg = NULL;
    if (tcb_deregister_swexn_handler(cur_tcb, &esp3, &eip, &arg) < 0)
        return -3;
    /* check if we should execute the previously deregistered handler */
    if (esp3 != NULL && eip != NULL){
        ureg_t *ureg_ptr = &(((ureg_t *)esp3)[-1]);
        *ureg_ptr = ureg;
        uint32_t *arg_ptr = (uint32_t *)(ureg_ptr) - 1;
        *arg_ptr = (uint32_t)(ureg_ptr);
        *(arg_ptr-1) = (uint32_t)arg;
        *(arg_ptr-2) = (uint32_t)(NULL);
        stack[ESP_IDX] = (uint32_t)(arg_ptr-2);
        stack[SS_IDX] = (uint32_t)SEGSEL_USER_DS;
        stack[EFLAGS_IDX] = (uint32_t)get_user_eflags();
        stack[CS_IDX] = (uint32_t)SEGSEL_USER_CS;
        stack[EIP_IDX] = (uint32_t)eip;
        return 0;
    }

    return -4;
}

void page_fault_c_handler(uint32_t *stack){
    /* attempt to execute swexn */
    if (swexn_execute(SWEXN_CAUSE_PAGEFAULT, stack) == 0)
        return;
    lprintf("Segmentation Fault");
    thr_set_status(-2);
    thr_vanish();
}

void double_fault_c_handler(void){
    lprintf("Double fault occured!");
    MAGIC_BREAK;
}
