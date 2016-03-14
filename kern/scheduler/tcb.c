/** @file tcb.c
 *
 */

#include <x86/seg.h>
/* page directory include */
#include <page_directory.h>
/* frame manager include */
#include <frame_manager.h>
#include <tcb.h>
#include <malloc.h>
#include <special_reg_cntrl.h>

#include <simics.h>
/*
typedef struct tcb{
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;

    uint32_t esi;
    uint32_t edi;

    uint32_t ebp;
    uint32_t esp;
    uint32_t eip;
    uint32_t e_flags;

    int pid;
    int tid;
    void *k_stack;

} tcb_t;
*/

int tcb_init(tcb_t *tcb, int tid, int pid, uint32_t s_top, uint32_t eip) {
    tcb->eax = 0;
    tcb->ebx = 0;
    tcb->ecx = 0;
    tcb->edx = 0;
    tcb->esi = 0;
    tcb->edi = 0;

    // TODO: Figure these out
    tcb->ebp = s_top;
    tcb->esp = s_top;
    tcb->eip = eip;
    tcb->e_flags = get_user_eflags();

    tcb->cs = SEGSEL_USER_CS;
    // TODO: Assign to proper value
    tcb->ss = SEGSEL_USER_DS;
    tcb->ds = SEGSEL_USER_DS;
    tcb->es = SEGSEL_USER_DS;
    tcb->fs = SEGSEL_USER_DS;
    tcb->gs = SEGSEL_USER_DS;

    /* These should be set by the scheduler */
    tcb->tid = tid;
    tcb->pid = pid;


    void *k_stack_bot = malloc(sizeof(void*) * PAGE_SIZE);
    if (k_stack_bot == NULL) return -1;

    tcb->k_stack = (void *)(((uint32_t) k_stack_bot) + PAGE_SIZE);

    return 0;
}

int tcb_destroy(tcb_t *tcb) {
    free(tcb->k_stack);
    // TODO: Clean up other shit
    return 0;

}

int tcb_gettid(tcb_t *tcb, int *tid){
    if (tcb == NULL) return -1;
    *tid = tcb->tid;
    return 0;
}

