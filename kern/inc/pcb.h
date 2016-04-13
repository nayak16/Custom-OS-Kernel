/** @file pcb_t.h
 *
 */

#ifndef _PCB_T_H_
#define _PCB_T_H_

/* page directory include */
#include <page_directory.h>
/* frame manager include */
#include <frame_manager.h>
/* semaphore type */
#include <sem.h>

/**
 * @brief A process control block that holds meta information for a process
 *
 */
typedef struct pcb{
    /** @brief unique id of the pcb */
    int pid;
    /** @brief pid of this pcb's parent pcb */
    int ppid;
    /** @brief the tid of the first thread in this process */
    int original_tid;
    /** @brief top of the user stack of this pcb */
    uint32_t stack_top;
    /** @brief Starting execution eip of this pcb's program */
    unsigned long entry_point;
    /** @brief Page directory of this pcb */
    page_directory_t pd;
    /** @brief Number of threads running in this pcb */
    uint32_t num_threads;
    /** @brief Number of child processes this pcb has */
    uint32_t num_child_proc;
    /** @brief Number of arguments to pcb's program entry point */
    int argc;
    /** @brief Array of string args to pcb's program entry point */
    char **argv;
    /**
     * @brief Semaphore that child processes signal to let the parent pcb know
     * it has vanished and to collect its status
     */
    sem_t wait_sem;
    /**
     * @brief Queue that child processes place themselves in to make themselves
     * available to the parent pcb
     */
    queue_t status_queue;
    /**
    * @brief Mutex is needed because other threads (not the current one) are
    * accessing and changing a pcb possibly simultaneously. For example,
    * in the case of a child modifying the parent. In some functions, like
    * pcb_load_prog, a lock is not necessary because only the current
    * running pcb should be loading a program into the itself.
    */
    mutex_t m;
} pcb_t;

int pcb_init(pcb_t *pcb);
int pcb_set_running(pcb_t *pcb);
int pcb_set_original_tid(pcb_t *pcb, int tid);
int pcb_get_original_tid(pcb_t *pcb, int *tid);
int pcb_destroy_s(pcb_t *pcb);
int pcb_load_prog(pcb_t *pcb, const char *filename, int argc, char** argv);
int pcb_copy(pcb_t *dest_pcb, pcb_t *source_pcb);
int pcb_wait_on_status(pcb_t *pcb, int *status_ptr, int *original_tid);
int pcb_signal_status(pcb_t *pcb, int status, int original_tid);
int pcb_get_ppid(pcb_t *pcb);

int pcb_inc_children_s(pcb_t *pcb);
int pcb_dec_children_s(pcb_t *pcb);

int pcb_inc_threads_s(pcb_t *pcb);
int pcb_dec_threads_s(pcb_t *pcb);

#endif /* _PCB_T_H_ */
