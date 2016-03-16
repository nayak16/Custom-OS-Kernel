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

int tcb_init(tcb_t *tcb, int tid, int pid, uint32_t *s_top, uint32_t eip) {

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
    k_stack_top[-1] = SEGSEL_USER_DS;
    k_stack_top[-2] = (uint32_t) s_top;
    k_stack_top[-3] = get_user_eflags();
    k_stack_top[-4] = SEGSEL_USER_CS;
    k_stack_top[-5] = eip;
    /* ---------- General Purpose Regs ---------- */
    k_stack_top[-6] = 0;   // eax
    k_stack_top[-7] = 0;   // ecx
    k_stack_top[-8] = 0;   // edx
    k_stack_top[-9] = 0;   // ebx
    k_stack_top[-10] = 0;  // skip esp
    k_stack_top[-11] = (uint32_t) s_top;  // ebp
    k_stack_top[-12] = 0;  // esi
    k_stack_top[-13] = 0;   // edi
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

