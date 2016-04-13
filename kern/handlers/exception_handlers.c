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
#include <stdbool.h>

#include <x86/cr.h>
#include <kern_internals.h>

#include <special_reg_cntrl.h>

#include <tcb.h>
#include <ureg.h>
#include <x86/seg.h>
#include <x86/idt.h>

#include <stdio.h>

/** @brief Given a stack pointer to the esp directly after the context has been
 *         saved, converts it into a ureg
 *  @param ureg The ureg to store to
 *  @param cause The cause of the exception
 *  @param stack The esp right after saving context
 *  @param generates_error_code Whether or not the particular exception
 *         generates an error code
 *  @return 0 on success, negative integer code on failure
 */
int ureg_from_stack(ureg_t *ureg, uint32_t cause, uint32_t *stack,
        bool generates_error_code){
    if (stack == NULL) return -1;
    ureg->cause = cause;
    ureg->cr2 = get_cr2();
    memcpy(&(ureg->ds), stack, REGS_SIZE * sizeof(unsigned int));
    if (!generates_error_code){
        memmove(&(ureg->eip), &(ureg->error_code), 5 * sizeof(unsigned int));
    }
    return 0;
}

/** @brief Executes a swexn if any exists for the current thread
 *  @param cause The cause of the exception
 *  @param stack The stack address directly after context has been saved
 *  @param generates_error_code Whether or not the particular exception
 *         generates an error code
 *  @return 0 on successful swexn execution, otherwise negative integer code
 */
int swexn_execute(uint32_t cause, uint32_t *stack, bool generates_error_code){
    /* get the current context */
    ureg_t ureg;
    if (ureg_from_stack(&ureg, cause, stack, generates_error_code) < 0)
        return -1;
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

        int offset;
        /* account for error code if it is not generated */
        if (generates_error_code){
            offset = 0;
        } else {
            offset = -1;
        }
        /* update the iret stack */
        stack[ESP_IDX+offset] = (uint32_t)(arg_ptr-2);
        stack[SS_IDX+offset] = (uint32_t)SEGSEL_USER_DS;
        stack[EFLAGS_IDX+offset] = (uint32_t)get_user_eflags();
        stack[CS_IDX+offset] = (uint32_t)SEGSEL_USER_CS;
        stack[EIP_IDX+offset] = (uint32_t)eip;
        return 0;
    }

    return -4;
}

/** @brief Given a cause, print to the console the reason for the exception
 *  @param cause The exception cause coe
 *  @return Void
 */
void exception_dump(int cause){
    tcb_t *cur_tcb;
    scheduler_get_current_tcb(&sched, &cur_tcb);
    char *reason;
    switch (cause){
        case IDT_DE:
            reason = "division error";
            break;
        case IDT_DB:
            reason = "debug exception";
            break;
        case IDT_BR:
            reason = "bound range exceeded";
            break;
        case IDT_UD:
            reason = "undefined opcode";
            break;
        case IDT_NM:
            reason = "no math coprocessor";
            break;
        case IDT_DF:
            reason = "double fault";
            break;
        case IDT_CSO:
            reason = "coprocessor segment overrun";
            break;
        case IDT_TS:
            reason = "invalid task segment selector";
            break;
        case IDT_NP:
            reason = "segment not present";
            break;
        case IDT_SS:
            reason = "stack segment fault";
            break;
        case IDT_GP:
            reason = "general protection fault";
            break;
        case IDT_PF:
            reason = "page fault";
            break;
        case IDT_MF:
            reason = "math fault";
            break;
        case IDT_AC:
            reason = "alignment check";
            break;
        case IDT_MC:
            reason = "machine check";
            break;
        case IDT_XF:
            reason = "floating point exception";
            break;
        default:
            reason = "unknown";
            break;
    }
    printf("Thread %d exited unexpectedly due to %s with status %d\n",
            cur_tcb->tid, reason, cur_tcb->exit_status);
}

/** @brief Prints all the registers to the console
 *  @param stack Pointer to the bottom of the saved context
 *  @return Void\
 */
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

/** @brief Prints cr2 to the console
 *  @return Void
 */
void print_cr2(){
    printf("cr2: %p\n", (void *)get_cr2());
}


/** @brief Implements the page fault handler
 *  @param stack The bottom address of the saved context
 *  @return Void
 */
void page_fault_c_handler(uint32_t *stack){
    /* attempt to execute swexn */
    if (swexn_execute(SWEXN_CAUSE_PAGEFAULT, stack, true) == 0)
        return;

    thr_set_status(-2);
    exception_dump(IDT_PF);
    print_cr2();
    register_dump(stack);
    thr_vanish();
}
/** @brief Implements the double fault handler
 *  @param stack The bottom address of the saved context
 *  @return Void
 */
void double_fault_c_handler(uint32_t *stack){
    exception_dump(IDT_DF);
    register_dump(stack);
}
/** @brief Implements the division error handler
 *  @param stack The bottom address of the saved context
 *  @return Void
 */
void division_error_c_handler(uint32_t *stack){
    if (swexn_execute(SWEXN_CAUSE_DIVIDE, stack, false) == 0)
        return;

    thr_set_status(-2);

    exception_dump(IDT_DE);
    register_dump(stack);
    thr_vanish();

}
/** @brief Implements the debug exception handler
 *  @param stack The bottom address of the saved context
 *  @return Void
 */
void debug_exception_c_handler(uint32_t *stack){
    if (swexn_execute(SWEXN_CAUSE_DEBUG, stack, false) == 0)
        return;

    thr_set_status(-2);

    exception_dump(IDT_DB);
    register_dump(stack);
    thr_vanish();

}
/** @brief Implements the breakpoint handler
 *  @param stack The bottom address of the saved context
 *  @return Void
 */
void breakpoint_c_handler(uint32_t *stack){
    if (swexn_execute(SWEXN_CAUSE_BREAKPOINT, stack, false) == 0)
        return;

    thr_set_status(-2);

    exception_dump(IDT_BP);
    register_dump(stack);
    thr_vanish();

}
/** @brief Implements the overflow handler
 *  @param stack The bottom address of the saved context
 *  @return Void
 */
void overflow_c_handler(uint32_t *stack){
    if (swexn_execute(SWEXN_CAUSE_OVERFLOW, stack, false) == 0)
        return;

    thr_set_status(-2);

    exception_dump(IDT_OF);
    register_dump(stack);
    thr_vanish();

}
/** @brief Implements the bound range handler
 *  @param stack The bottom address of the saved context
 *  @return Void
 */
void bound_range_c_handler(uint32_t *stack){
    if (swexn_execute(SWEXN_CAUSE_BOUNDCHECK, stack, false) == 0)
        return;

    thr_set_status(-2);

    exception_dump(IDT_BR);
    register_dump(stack);
    thr_vanish();

}
/** @brief Implements the undefined op handler
 *  @param stack The bottom address of the saved context
 *  @return Void
 */
void undef_op_c_handler(uint32_t *stack){
    if (swexn_execute(SWEXN_CAUSE_OPCODE, stack, false) == 0)
        return;

    thr_set_status(-2);

    exception_dump(IDT_UD);
    register_dump(stack);
    thr_vanish();

}
/** @brief Implements the no math handler
 *  @param stack The bottom address of the saved context
 *  @return Void
 */
void no_math_c_handler(uint32_t *stack){
    if (swexn_execute(SWEXN_CAUSE_NOFPU, stack, false) == 0)
        return;
    thr_set_status(-2);

    exception_dump(IDT_NM);
    register_dump(stack);
    thr_vanish();

}

/** @brief Implements the cso handler
 *  @param stack The bottom address of the saved context
 *  @return Void
 */
void coprocessor_segment_overrun_c_handler(uint32_t *stack){
    thr_set_status(-2);

    exception_dump(IDT_CSO);
    register_dump(stack);
    thr_vanish();

}
/** @brief Implements the invalid tss handler
 *  @param stack The bottom address of the saved context
 *  @return Void
 */
void invalid_tss_c_handler(uint32_t *stack){
    thr_set_status(-2);

    exception_dump(IDT_TS);
    register_dump(stack);
    thr_vanish();

}
/** @brief Implements the segment not present handler
 *  @param stack The bottom address of the saved context
 *  @return Void
 */
void segment_not_present_c_handler(uint32_t *stack){
    if (swexn_execute(SWEXN_CAUSE_SEGFAULT, stack, true) == 0)
        return;

    thr_set_status(-2);

    exception_dump(IDT_NP);
    register_dump(stack);
    thr_vanish();

}
/** @brief Implements the ss fault handler
 *  @param stack The bottom address of the saved context
 *  @return Void
 */
void ss_fault_c_handler(uint32_t *stack){
    if (swexn_execute(SWEXN_CAUSE_STACKFAULT, stack, true) == 0)
        return;

    thr_set_status(-2);

    exception_dump(IDT_SS);
    register_dump(stack);
    thr_vanish();

}
/** @brief Implements the gp fault handler
 *  @param stack The bottom address of the saved context
 *  @return Void
 */
void gp_fault_c_handler(uint32_t *stack){
    if (swexn_execute(SWEXN_CAUSE_PROTFAULT, stack, true) == 0)
        return;

    thr_set_status(-2);

    exception_dump(IDT_GP);
    register_dump(stack);
    thr_vanish();

}
/** @brief Implements the math fault handler
 *  @param stack The bottom address of the saved context
 *  @return Void
 */
void math_fault_c_handler(uint32_t *stack){
    if (swexn_execute(SWEXN_CAUSE_FPUFAULT, stack, false) == 0)
        return;

    thr_set_status(-2);

    exception_dump(IDT_MF);
    register_dump(stack);
    thr_vanish();

}
/** @brief Implements the align fault handler
 *  @param stack The bottom address of the saved context
 *  @return Void
 */
void align_fault_c_handler(uint32_t *stack){
    if (swexn_execute(SWEXN_CAUSE_ALIGNFAULT, stack, true) == 0)
        return;

    thr_set_status(-2);

    exception_dump(IDT_AC);
    register_dump(stack);
    thr_vanish();

}
/** @brief Implements the machine check handler
 *  @param stack The bottom address of the saved context
 *  @return Void
 */
void machine_check_fault_c_handler(uint32_t *stack){
    exception_dump(IDT_MC);
    register_dump(stack);
    thr_vanish();
}
/** @brief Implements the simd handler
 *  @param stack The bottom address of the saved context
 *  @return Void
 */
void simd_fault_c_handler(uint32_t *stack){
    if (swexn_execute(SWEXN_CAUSE_SIMDFAULT, stack, false) == 0)
        return;

    thr_set_status(-2);

    exception_dump(IDT_XF);
    register_dump(stack);
    thr_vanish();
}

