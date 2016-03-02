/** @file tcb.h
 *
 */

#ifndef _TCB_H_
#define _TCB_H_

/* page directory include */
#include <page_directory.h>
/* frame manager include */
#include <frame_manager.h>

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

int tcb_init(tcb_t *tcb);
int tcb_destroy(tcb_t *tcb);

#endif /* _TCB_H_ */
