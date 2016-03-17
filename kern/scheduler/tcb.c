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

#define SS_IDX 15
#define ESP_IDX 14
#define EFLAGS_IDX 13
#define CS_IDX 12
#define EIP_IDX 11
#define ECX_IDX 10
#define EDX_IDX 9
#define EBX_IDX 8
#define EBP_IDX 7
#define ESI_IDX 6
#define EDI_IDX 5
#define DS_IDX 3
#define ES_IDX 2
#define FS_IDX 1
#define GS_IDX 0

int tcb_init(tcb_t *tcb, int tid, int pid,
             uint32_t *s_top, uint32_t eip, uint32_t *regs) {

    /* These should be set by the scheduler */
    tcb->id = tid;
    tcb->pid = pid;


    /* Save final esp */
    tcb->esp = (uint32_t) s_top;

    void *k_stack_bot = malloc(sizeof(void*) * PAGE_SIZE);
    if (k_stack_bot == NULL) return -1;

    uint32_t* k_stack_top = (uint32_t*)(((uint32_t) k_stack_bot) + PAGE_SIZE);

    /* Push meta data to stack */
    /* ------------- IRET section --------------- */
    k_stack_top[-1] = regs == NULL ? SEGSEL_USER_DS : regs[SS_IDX];
    k_stack_top[-2] = regs == NULL ? (uint32_t) s_top : regs[ESP_IDX];
    k_stack_top[-3] = regs == NULL ? get_user_eflags() : regs[EFLAGS_IDX];
    k_stack_top[-4] = regs == NULL ? SEGSEL_USER_CS : regs[CS_IDX];
    k_stack_top[-5] = regs == NULL ? eip : regs[EIP_IDX];
    /* ---------- General Purpose Regs ---------- */
    k_stack_top[-6] = 0;   // eax
    k_stack_top[-7] = regs == NULL ? 0 : regs[ECX_IDX];   // ecx
    k_stack_top[-8] = regs == NULL ? 0 : regs[EDX_IDX];   // edx
    k_stack_top[-9] = regs == NULL ? 0 : regs[EBX_IDX];   // ebx
    k_stack_top[-10] = 0;  // skip esp
    k_stack_top[-11] = regs == NULL ? (uint32_t) s_top : regs[EBP_IDX];  // ebp
    k_stack_top[-12] = regs == NULL ? 0 : regs[ESI_IDX];  // esi
    k_stack_top[-13] = regs == NULL ? 0 : regs[EDI_IDX];  // edi
    /* --------- Extra Segment Selectors -------- */
    k_stack_top[-14] = SEGSEL_USER_DS; // ds
    k_stack_top[-15] = SEGSEL_USER_DS; // es
    k_stack_top[-16] = SEGSEL_USER_DS; // fs
    k_stack_top[-17] = SEGSEL_USER_DS; // gs

    tcb->k_stack = (void *)(&(k_stack_top[-17]));
    return 0;
}

int tcb_destroy(tcb_t *tcb) {
    free(tcb->k_stack);
    // TODO: Clean up other shit
    return 0;

}

int tcb_gettid(tcb_t *tcb, int *tid){
    if (tcb == NULL) return -1;
    *tid = tcb->id;
    return 0;
}

