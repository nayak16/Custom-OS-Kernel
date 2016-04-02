/** @file tcb.h
 *
 */

#ifndef _TCB_H_
#define _TCB_H_

/* page directory include */
#include <page_directory.h>
/* frame manager include */
#include <frame_manager.h>

#include <pcb.h>

/* possible tcb statuses */
#define UNINIT 0
#define RUNNABLE 1
#define WAITING 2
#define ZOMBIE 3
#define RUNNING 4
#define SLEEPING 5

#define REGS_SIZE 17

#define SS_IDX 16
#define ESP_IDX 15
#define EFLAGS_IDX 14
#define CS_IDX 13
#define EIP_IDX 12
// Skip EAX reg 11
#define ECX_IDX 10
#define EDX_IDX 9
#define EBX_IDX 8
// Skip ESP reg 7
#define EBP_IDX 6
#define ESI_IDX 5
#define EDI_IDX 4
#define DS_IDX 3
#define ES_IDX 2
#define FS_IDX 1
#define GS_IDX 0


typedef struct tcb{

    int tid;

    int status;
    int exit_status;
    uint32_t t_wakeup;

    pcb_t *pcb;

    uint32_t *k_stack_bot;
    uint32_t *orig_k_stack;
    uint32_t *tmp_k_stack;

} tcb_t;

int tcb_init(tcb_t *tcb, int tid, pcb_t *pcb, uint32_t *regs);
int tcb_get_pcb(tcb_t *tcb, pcb_t **pcb);
int tcb_get_init_stack(tcb_t *tcb, void **stack);
int tcb_t_wakeup_cmp(void *a, void *b);

int tcb_reload_safe(tcb_t *tcb, pcb_t *pcb);
int tcb_get_status(tcb_t *tcb, int *statusp);

int tcb_destroy(tcb_t *tcb);
int tcb_gettid(tcb_t *tcb, int *tid);
int tcb_get_exit_status(tcb_t *tcb, int *status);

#endif /* _TCB_H_ */
