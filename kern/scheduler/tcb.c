/** @file tcb.c
 *
 */

/* page directory include */
#include <page_directory.h>
/* frame manager include */
#include <frame_manager.h>
#include <tcb.h>
#include <malloc.h>

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

    int pid;
    int tid;
    void *k_stack;

} tcb_t;
*/

int tcb_init(tcb_t *tcb) {
    tcb->eax = 0;
    tcb->ebx = 0;
    tcb->ecx = 0;
    tcb->edx = 0;
    tcb->esi = 0;
    tcb->edi = 0;

    // TODO: Figure these out
    tcb->ebp = 0;
    tcb->esp = 0;

    /* These should be set by the scheduler */
    tcb->tid = -1;
    tcb->pid = -1;
    // TODO: Figure this out
    tcb->k_stack = malloc(PAGE_SIZE);
    if (tcb->k_stack == NULL) return -1;

    return 0;
}

int tcb_destroy(tcb_t *tcb) {
    free(tcb->k_stack);
    // TODO: Clean up other shit
    return 0;

}

