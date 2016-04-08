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
#include <x86/idt.h>

#include <stdio.h>

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

void exception_dump(int cause){
    tcb_t *cur_tcb;
    scheduler_get_current_tcb(&sched, &cur_tcb);
    char *reason;
    switch (cause){
        case IDT_PF:
            reason = "page fault";
            break;
        default:
            reason = "unknown";
            break;
    }
    printf("Thread %d exited unexpectedly due to %s with status %d\n",
            cur_tcb->tid, reason, cur_tcb->exit_status);
}

void register_dump(uint32_t *stack){
    printf("------ Context ------\n\
    ss:     0x%x\n\
    eflags: 0x%x\n\
    cs:     0x%x\n\
    eip:    0x%x\n\
    eax:    0x%x\n\
    ebx:    0x%x\n\
    ecx:    0x%x\n\
    edx:    0x%x\n\
    esp:    0x%x\n\
    ebp:    0x%x\n\
    esi:    0x%x\n\
    edi:    0x%x\n\
    ds:     0x%x\n\
    es:     0x%x\n\
    fs:     0x%x\n\
    gs:     0x%x\n\
    ------ End Context -------\n",
    (unsigned int)stack[SS_IDX],
    (unsigned int)stack[EFLAGS_IDX],
    (unsigned int)stack[CS_IDX],
    (unsigned int)stack[EIP_IDX],
    (unsigned int)stack[EAX_IDX],
    (unsigned int)stack[EBX_IDX],
    (unsigned int)stack[ECX_IDX],
    (unsigned int)stack[EDX_IDX],
    (unsigned int)stack[ESP_IDX],
    (unsigned int)stack[EBP_IDX],
    (unsigned int)stack[ESI_IDX],
    (unsigned int)stack[EDI_IDX],
    (unsigned int)stack[DS_IDX],
    (unsigned int)stack[ES_IDX],
    (unsigned int)stack[FS_IDX],
    (unsigned int)stack[GS_IDX]);

}

void page_fault_c_handler(uint32_t *stack){
    /* attempt to execute swexn */
    if (swexn_execute(SWEXN_CAUSE_PAGEFAULT, stack) == 0)
        return;

    thr_set_status(-2);

    exception_dump(IDT_PF);
    register_dump(stack);
    thr_vanish();
}

void double_fault_c_handler(uint32_t *stack){
    exception_dump(IDT_DF);
    register_dump(stack);
    MAGIC_BREAK;
}


