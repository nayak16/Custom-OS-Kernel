/** @file tcb.c
 *
 *  @brief Implementation of thread control block functions
 *
 *  @author Aatish Nayak (aatishn@andrew.cmu.edu)
 *  @author Christopher Wei (cjwei@andrew.cmu.edu)
 *
 */

#include <x86/seg.h>
/* page directory include */
#include <page_directory.h>
/* frame manager include */
#include <frame_manager.h>
#include <kern_internals.h>
#include <tcb.h>
#include <malloc.h>
#include <special_reg_cntrl.h>
#include <x86/asm.h>

/**
 * @brief Loads the specified kstack with the appropriate values provided.
 *
 * @param tcb tcb to use to set kstack values
 * @param pcb pcb to get stack_top and entry_point from
 * @param k_stack_top top of k_stack to set registers and meta data
 * @param regs array of preset registers (NULL to fill with default)
 *
 */
void load_kstack(tcb_t *tcb, pcb_t *pcb,
                uint32_t *k_stack_top, uint32_t *regs) {

    /* Push meta data to stack */
    /* ------------- IRET section --------------- */
    k_stack_top[-1] = regs == NULL ? SEGSEL_USER_DS : regs[SS_IDX];
    k_stack_top[-2] = regs == NULL ? (uint32_t) pcb->stack_top : regs[ESP_IDX];
    k_stack_top[-3] = regs == NULL ? get_user_eflags() : regs[EFLAGS_IDX];
    k_stack_top[-4] = regs == NULL ? SEGSEL_USER_CS : regs[CS_IDX];
    k_stack_top[-5] = regs == NULL ? pcb->entry_point : regs[EIP_IDX];
    k_stack_top[-6] = 0;
    /* ---------- General Purpose Regs ---------- */
    k_stack_top[-7] = 0;
    k_stack_top[-8] = regs == NULL ? 0 : regs[ECX_IDX];
    k_stack_top[-9] = regs == NULL ? 0 : regs[EDX_IDX];
    k_stack_top[-10] = regs == NULL ? 0 : regs[EBX_IDX];
    k_stack_top[-11] = 0;  // skip esp
    k_stack_top[-12] = regs == NULL ? (uint32_t) pcb->stack_top : regs[EBP_IDX];
    k_stack_top[-13] = regs == NULL ? 0 : regs[ESI_IDX];
    k_stack_top[-14] = regs == NULL ? 0 : regs[EDI_IDX];
    /* --------- Extra Segment Selectors -------- */
    k_stack_top[-15] = regs == NULL ? SEGSEL_USER_DS : regs[DS_IDX];
    k_stack_top[-16] = regs == NULL ? SEGSEL_USER_DS : regs[ES_IDX];
    k_stack_top[-17] = regs == NULL ? SEGSEL_USER_DS : regs[FS_IDX];
    k_stack_top[-18] = regs == NULL ? SEGSEL_USER_DS : regs[GS_IDX];

    tcb->k_stack_top = k_stack_top;
    tcb->orig_k_stack = (void *)(&(k_stack_top[-18]));
    tcb->tmp_k_stack = tcb->orig_k_stack;
}

/**
 * @brief Initializes a tcb with appropriate values
 *
 * @param tcb tcb to initialize
 * @param tid tid of tcb to initialize
 * @param pcb pcb of the tcb being initialized
 * @param regs array of preset registers (NULL to just use default values)
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int tcb_init(tcb_t *tcb, int tid, pcb_t *pcb, uint32_t *regs) {
    if (tcb == NULL || pcb == NULL) return -1;

    /* Set appropriate tid and pcb */
    tcb->tid = tid;
    tcb->pcb = pcb;

    /* Set tcb to runnable */
    tcb->status = RUNNABLE;

    /* Init a k_stack which will also be used for scheduling */
    tcb->k_stack_bot = malloc(8*PAGE_SIZE);
    if (tcb->k_stack_bot == NULL) return -2;

    /* Calculate stack_top */
    uint32_t* k_stack_top = (uint32_t*)(((uint32_t) tcb->k_stack_bot) + 8*PAGE_SIZE);

    /* Load kstack with appropriate values */
    load_kstack(tcb, pcb, k_stack_top, regs);

    tcb->swexn_handler = NULL;
    tcb->swexn_handler_arg = NULL;
    tcb->swexn_handler_esp = NULL;

    return 0;
}

/**
 * @brief Obtains the starting k_stack with metadata properly setup
 *
 * @param tcb tcb to access
 * @param stack address to place the k_stack at
 *
 * @return 0 on success, -1 on error
 *
 */
int tcb_get_init_stack(tcb_t *tcb, void **stack) {
    if (tcb == NULL || stack == NULL) return -1;

    *stack = tcb->orig_k_stack;
    return 0;
}

/**
 * @brief Destroys a tcb
 *
 * Frees the k_stack tcb
 *
 * @param tcb tcb to access
 *
 */
void tcb_destroy(tcb_t *tcb) {
    free(tcb->k_stack_bot);
}

/**
 * @brief Obtains the corresponding pcb of the specified tcb
 *
 * @param tcb specified tcb to get pcb from
 * @param pcbp address to place pcb
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int tcb_get_pcb(tcb_t *tcb, pcb_t **pcbp) {
    if (tcb == NULL || pcbp == NULL) return -1;
    *pcbp = tcb->pcb;
    return 0;
}

/**
 * @brief Reloads the specified tcb with new k_stack values
 *
 * Also, deregisters any swexn handlers
 *
 * @param tcb tcb to reload
 * @param pcb pcb to obtain new values from (stack and entry point)
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int tcb_reload(tcb_t *tcb, pcb_t *pcb) {
    if (tcb == NULL || pcb == NULL) return -1;

    /* Load k_stack with new values */
    load_kstack(tcb, pcb, tcb->k_stack_top, NULL);

    /* Deregister swexn handler */
    tcb_deregister_swexn_handler(tcb, NULL, NULL, NULL);

    return 0;
}

/**
 * @brief Compares wakeup times of two tcbs.
 *
 * Used in inserting a tcb into a linked list
 *
 * @param a first tcb to compare
 * @param b second tcb to compare
 *
 * @return 0 - a wakeup time = b wakeup time
 *        -1 - a wakeup time > b wakeup time
 *         1 - a wakeup time < b wakeup time
 *
 *
 */
int tcb_t_wakeup_cmp(void *a, void *b){
    uint32_t t1 = (uint32_t)((tcb_t *)a)->t_wakeup;
    uint32_t t2 = (uint32_t)((tcb_t *)b)->t_wakeup;
    return (t1 < t2) ? -1 : ((t1 > t2) ? 1 : 0);
}

/**
 * @brief Gets the exit status of a tcb
 *
 * @param tcb tcb to get exit status of
 * @param status_ptr address to place exit status at
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int tcb_get_exit_status(tcb_t *tcb, int *status_ptr){
    if (tcb == NULL) return -1;
    *status_ptr = tcb->exit_status;
    return 0;
}

/**
 * @brief Deregisters a swexn handler from the specified tcb and saves
 * previously registered values in provided pointers
 *
 * @param tcb tcb to deregister from
 * @param esp3 address to store deregistered esp3
 * @param eip address to store deregistered eip
 * @param arg address to store deregistered arg
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int tcb_deregister_swexn_handler(tcb_t *tcb, void **esp3,
        void (**eip)(void *arg, ureg_t *ureg), void **arg){
    if (tcb == NULL) return -1;
    if (esp3 != NULL) *esp3 = tcb->swexn_handler_esp;
    tcb->swexn_handler_esp = NULL;
    if (arg != NULL) *arg = tcb->swexn_handler_arg;
    tcb->swexn_handler_arg = NULL;
    if (eip != NULL) *eip = tcb->swexn_handler;
    tcb->swexn_handler = NULL;
    return 0;
}

/**
 * @brief Registers a swexn handler to the specified tcb
 *
 * @param tcb tcb to register to
 * @param esp3 exception stack to register
 * @param eip address of handler to register
 * @param arg address of arguments to handler
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int tcb_register_swexn_handler(tcb_t *tcb, void *esp3,
    void (*eip)(void *arg, ureg_t *ureg), void *arg){
    if (tcb == NULL) return -1;
    tcb->swexn_handler_esp = esp3;
    tcb->swexn_handler_arg = arg;
    tcb->swexn_handler = eip;
    return 0;
}

