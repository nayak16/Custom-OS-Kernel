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
    /************************************************************************
     * WARNING: changes in our typedef will mess up assembly functions that *
     * rely on the layout of tcb                                            *
     ************************************************************************/

    int id;
    uint32_t eax; /* Side note: lowest stack addr when pushed */
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;

    uint32_t esi;
    uint32_t edi;


    /* Special Registers */
    uint32_t ebp;
    uint32_t e_flags;
    uint32_t ss;
    uint32_t cs;
    uint32_t eip;
    uint32_t esp;

    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t gs;

    int pid;

    void *k_stack;

} tcb_t;

int tcb_init(tcb_t *tcb, int tid, int pid, uint32_t *s_top, uint32_t eip);
int tcb_destroy(tcb_t *tcb);
int tcb_gettid(tcb_t *tcb, int *tid);

#endif /* _TCB_H_ */
