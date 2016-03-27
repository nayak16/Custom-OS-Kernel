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
#include <x86/asm.h>

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

#define SS_IDX 14
#define ESP_IDX 13
#define EFLAGS_IDX 12
#define CS_IDX 11
#define EIP_IDX 10
#define ECX_IDX 9
#define EDX_IDX 8
#define EBX_IDX 7
#define EBP_IDX 6
#define ESI_IDX 5
#define EDI_IDX 4
#define DS_IDX 3
#define ES_IDX 2
#define FS_IDX 1
#define GS_IDX 0

int tcb_init(tcb_t *tcb, int tid, pcb_t *pcb, uint32_t *regs) {

    /* Set appropriate tid and pcb */
    tcb->tid = tid;
    tcb->pcb = pcb;
    pcb->num_threads++;

    /* Set tcb to runnable */
    tcb->status = RUNNABLE;

    /* Init a k_stack which will also be used for scheduling */
    tcb->k_stack_bot = malloc(sizeof(void*) * PAGE_SIZE);
    if (tcb->k_stack_bot == NULL) return -1;

    uint32_t* k_stack_top = (uint32_t*)(((uint32_t) tcb->k_stack_bot) + PAGE_SIZE);

    /* Push meta data to stack */
    /* ------------- IRET section --------------- */
    k_stack_top[-1] = regs == NULL ? SEGSEL_USER_DS : regs[SS_IDX];
    k_stack_top[-2] = regs == NULL ? (uint32_t) pcb->stack_top : regs[ESP_IDX];
    k_stack_top[-3] = regs == NULL ? get_user_eflags() : regs[EFLAGS_IDX];
    k_stack_top[-4] = regs == NULL ? SEGSEL_USER_CS : regs[CS_IDX];
    k_stack_top[-5] = regs == NULL ? pcb->entry_point : regs[EIP_IDX];
    /* ---------- General Purpose Regs ---------- */
    k_stack_top[-6] = 0;   // eax
    k_stack_top[-7] = regs == NULL ? 0 : regs[ECX_IDX];   // ecx
    k_stack_top[-8] = regs == NULL ? 0 : regs[EDX_IDX];   // edx
    k_stack_top[-9] = regs == NULL ? 0 : regs[EBX_IDX];   // ebx
    k_stack_top[-10] = 0;  // skip esp
    k_stack_top[-11] = regs == NULL ? (uint32_t) pcb->stack_top : regs[EBP_IDX];  // ebp
    k_stack_top[-12] = regs == NULL ? 0 : regs[ESI_IDX];  // esi
    k_stack_top[-13] = regs == NULL ? 0 : regs[EDI_IDX];  // edi
    /* --------- Extra Segment Selectors -------- */
    k_stack_top[-14] = SEGSEL_USER_DS; // ds
    k_stack_top[-15] = SEGSEL_USER_DS; // es
    k_stack_top[-16] = SEGSEL_USER_DS; // fs
    k_stack_top[-17] = SEGSEL_USER_DS; // gs

    tcb->orig_k_stack = (void *)(&(k_stack_top[-17]));
    tcb->tmp_k_stack = tcb->orig_k_stack;
    return 0;
}

int tcb_get_init_stack(tcb_t *tcb, void **stack) {
    if (tcb == NULL || stack == NULL) return -1;

    *stack = tcb->orig_k_stack;
    return 0;
}

int tcb_destroy(tcb_t *tcb) {
    tcb->pcb->num_threads--;
    free(tcb->k_stack_bot);
    return 0;
}

int tcb_get_pcb(tcb_t *tcb, pcb_t **pcb) {
    if (tcb == NULL || pcb == NULL) return -1;
    *pcb = tcb->pcb;
    return 0;
}

int tcb_gettid(tcb_t *tcb, int *tid){
    if (tcb == NULL) return -1;
    *tid = tcb->tid;
    return 0;
}

int tcb_get_status(tcb_t *tcb, int *statusp){
    if (tcb == NULL || statusp == NULL) return -1;
    // TODO: Make atomic??
    *statusp = tcb->status;
    return 0;
}

int tcb_reload_safe(tcb_t *tcb, pcb_t *pcb) {
    tcb_destroy(tcb);
    tcb_init(tcb, tcb->tid, pcb, NULL);
    return 0;
}

int tcb_t_wakeup_cmp(void *a, void *b){
    uint32_t t1 = (uint32_t)((tcb_t *)a)->t_wakeup;
    uint32_t t2 = (uint32_t)((tcb_t *)b)->t_wakeup;
    return (t1 < t2) ? -1 : ((t1 > t2) ? 1 : 0);
}
