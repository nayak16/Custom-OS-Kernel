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

#include <ureg.h>
/* possible tcb statuses */
#define UNINIT 0
#define RUNNABLE 1
#define WAITING 2
#define ZOMBIE 3
#define RUNNING 4
#define SLEEPING 5

#define REGS_SIZE 18

#define SS_IDX 17
#define ESP_IDX 16
#define EFLAGS_IDX 15
#define CS_IDX 14
#define EIP_IDX 13
#define ERRCODE_IDX 12

#define EAX_IDX 11
#define ECX_IDX 10
#define EDX_IDX 9
#define EBX_IDX 8
// Skip ESP reg 7
#define EBP_IDX 6
#define ESI_IDX 5
#define EDI_IDX 4
#define GS_IDX 3
#define FS_IDX 2
#define ES_IDX 1
#define DS_IDX 0

/**
 * @brief A thread control block that holds meta information for a thread
 *
 */
typedef struct tcb{

    /** @brief unique id of this tcb */
    int tid;
    /**
     * @brief Current scheduler status of this tcb (See above for possible
     * statuses)
     */
    int status;
    /** @brief Exit status of this thread. Set with a call to set_status */
    int exit_status;
    /**
     * @brief If this thread is sleeping, t_wakeup is absolute time to wake up
     * at. Measured in number of ticks
     */
    uint32_t t_wakeup;
    /**
     * @brief The pcb that this tcb is running under. Multiple tcbs can have
     * the same pcb (multi-threaded)
     */
    pcb_t *pcb;

    /**
     * @brief Bottom of this tcb's k_stack
     */
    uint32_t *k_stack_bot;

    /**
     * @brief Top of this tcb's k_stack
     */
    uint32_t *k_stack_top;

    /**
     * @brief Address of the last meta/context data that was pushed onto the
     * k_stack. This is used as a starting point for entering a program.
     *
     * k_stack_bot < orig_k_stack < k_stack_top
     */
    uint32_t *orig_k_stack;

    /**
     * @brief Temporary k_stack address that context information is pushed
     * onto on every context switch. The tmp_k_stack is saved everytime so a
     * thread can be restored when context switched back into.
     */
    uint32_t *tmp_k_stack;

    /**
     * @brief Generic exception handler for this tcb installed via a call to
     * swexn
     */
    void (*swexn_handler)(void *arg, ureg_t *ureg);

    /**
     * @brief Pointer to the args of the installed exception handler
     */
    void *swexn_handler_arg;
    /**
     * @brief Top of the exception stack to use when calling the installed
     * exception handler
     */
    void *swexn_handler_esp;
    /**
     * @brief Pointer to the ureg struct that holds the context when the
     * exception occured. Passed into the swexn handler as an argument
     */
    ureg_t *swexn_ureg;
} tcb_t;

int tcb_init(tcb_t *tcb, int tid, pcb_t *pcb, uint32_t *regs);
int tcb_get_pcb(tcb_t *tcb, pcb_t **pcb);
int tcb_get_init_stack(tcb_t *tcb, void **stack);
int tcb_t_wakeup_cmp(void *a, void *b);
int tcb_reload(tcb_t *tcb, pcb_t *pcb);
void tcb_destroy(tcb_t *tcb);
int tcb_get_exit_status(tcb_t *tcb, int *status);
int tcb_deregister_swexn_handler(tcb_t *tcb, void **esp3,
        void (**eip)(void *arg, ureg_t *ureg), void **arg);
int tcb_register_swexn_handler(tcb_t *tcb, void *esp3,
        void (*eip)(void *arg, ureg_t *ureg), void *arg);

#endif /* _TCB_H_ */
