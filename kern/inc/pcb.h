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

typedef struct pcb{
    int pid;
    int ppid;
    int original_tid;
    uint32_t stack_top;

    int argc;
    char **argv;

    unsigned long entry_point;
    page_directory_t pd;

    uint32_t num_threads;
    uint32_t num_child_proc;
    sem_t wait_sem;
    queue_t status_queue;
} pcb_t;

int pcb_init(pcb_t *pcb);
int pcb_set_running(pcb_t *pcb);
int pcb_set_original_tid(pcb_t *pcb, int tid);
int pcb_get_original_tid(pcb_t *pcb, int *tid);
int pcb_destroy(pcb_t *pcb);
int pcb_load_prog(pcb_t *pcb, const char *filename, int argc, char** argv);
int pcb_copy(pcb_t *dest_pcb, pcb_t *source_pcb);
int pcb_wait_on_status(pcb_t *pcb, int *status_ptr, int *original_tid);
int pcb_signal_status(pcb_t *pcb, int status, int original_tid);
int pcb_get_ppid(pcb_t *pcb);

int pcb_inc_children(pcb_t *pcb);
int pcb_dec_children(pcb_t *pcb);

int pcb_inc_threads(pcb_t *pcb);
int pcb_dec_threads(pcb_t *pcb);

int pcb_get_child_count(pcb_t *pcb);

#endif /* _PCB_T_H_ */
