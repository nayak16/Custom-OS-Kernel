/** @file pcb_t.h
 *
 */

#include <stdlib.h>
#include <string.h>

#include <constants.h>
#include <pcb.h>
#include <queue.h>
#include <kern_internals.h>
/* set_pdbr */
#include <special_reg_cntrl.h>

/* get_bytes */
#include <loader.h>
#include <mem_section.h>


/* PAGE_SIZE */
#include <x86/page.h>
#include <elf_410.h>
#include <loader.h>
#include <sem.h>

#include <debug.h>
#include <simics.h>

#include <virtual_mem_mgmt.h>

int pcb_init(pcb_t *pcb){
    if (pcb == NULL) return -1;
    /* Temp value before being added to scheduler */
    pcb->pid = -1;
    pcb->ppid = -1;
    pcb->original_tid = -1;
    pcb->num_child_proc = 0;
    pcb->num_threads = 0;

    /* Initialize an empty queue; use semaphore to allocate resources
     * when they become avaliable */
    mutex_init(&(pcb->m));
    queue_init(&(pcb->status_queue));
    sem_init(&(pcb->wait_sem), 0);
    pd_init(&(pcb->pd));
    return 0;
}

int pcb_destroy_safe(pcb_t *pcb){
    if (pcb == NULL) return -1;

    mutex_lock(&(pcb->m));
    sem_destroy(&(pcb->wait_sem));
    queue_destroy(&(pcb->status_queue));
    vmm_clear_user_space(&(pcb->pd));
    pd_destroy(&(pcb->pd));
    mutex_unlock(&(pcb->m));

    mutex_destroy(&(pcb->m));
    return 0;
}

int pcb_copy(pcb_t *dest_pcb, pcb_t *source_pcb) {
    if(source_pcb == NULL || dest_pcb == NULL) return -1;

    /* Set ppid of dest_pcb to pid of source_pcb */
    dest_pcb->ppid = source_pcb->pid;

    /* Copy address space */
    if(vmm_deep_copy(&(dest_pcb->pd)) < 0) {
        return -2;
    }

    return 0;
}


int pcb_load_prog(pcb_t *pcb, const char *filename, int argc, char** argv){
    if (pcb == NULL || filename == NULL) return -1;
    simple_elf_t elf;
    if (elf_check_header(filename) != ELF_SUCCESS) return -2;
    if (elf_load_helper(&elf, filename) != ELF_SUCCESS) return -3;

    /* Load elf sections into memory */
    if (load_elf_sections(&elf, pcb) < 0) return -4;

    /* Save argc and argv */
    pcb->argc = argc;
    pcb->argv = argv;

    /* Load user stack into memory */
    if (load_user_stack(pcb) < 0) return -5;

    return 0;
}

typedef struct pcb_metadata{
    int status;
    int original_tid;
} pcb_metadata_t;

int pcb_signal_status(pcb_t *pcb, int status, int original_tid){
    if (pcb == NULL) return -1;
    /* Create struct to hold meta data */
    pcb_metadata_t *metadata = malloc(sizeof(pcb_metadata_t));
    if (metadata == NULL) {
        mutex_unlock(&(pcb->m));
        return -2;
    }
    metadata->status = status;
    metadata->original_tid = original_tid;

    lprintf("TCB %d signaling parent_pcb %d", sched.cur_tcb->tid, pcb->pid);
    /* Put status to collect into queue */
    if (queue_enq(&(pcb->status_queue), (void *) metadata) < 0) {
        mutex_unlock(&(pcb->m));
        return -2;
    }

    /* Signal that a status is available */
    sem_signal(&(pcb->wait_sem));
    return 0;
}

int pcb_wait_on_status(pcb_t *pcb, int *status_ptr, int *original_pid){
    if (pcb == NULL) return -1;
    /* Check if there's any child processes left to wait on */
    if (pcb->num_child_proc == 0) {
        return -2;
    }

    /* Wait on available statuses */
    sem_wait(&(pcb->wait_sem));

    mutex_lock(&(pcb->m));
    /* Get next available status form queue */
    pcb_metadata_t *metadata;
    if (queue_deq(&(pcb->status_queue), (void **)&metadata) < 0)
        return -2;

    /* Extract status ptr and orig pid */
    if (status_ptr != NULL) *status_ptr = metadata->status;
    if (original_pid != NULL) *original_pid = metadata->original_tid;

    mutex_unlock(&(pcb->m));
    /* Free metadata struct */
    free(metadata);
    return 0;
}

int pcb_get_pid(pcb_t *pcb){
    if (pcb == NULL) return -1;
    return pcb->pid;
}

int pcb_get_ppid(pcb_t *pcb){
    if (pcb == NULL) return -1;
    return pcb->ppid;
}


int pcb_set_original_tid(pcb_t *pcb, int tid){
    if (pcb == NULL) return -1;
    pcb->original_tid = tid;
    return 0;
}

int pcb_get_original_tid(pcb_t *pcb, int *tid){
    if (pcb == NULL || tid == NULL) return -1;
    *tid = pcb->original_tid;
    return 0;
}

int pcb_inc_children(pcb_t *pcb) {
    if (pcb == NULL) return -1;
    mutex_lock(&(pcb->m));
    pcb->num_child_proc++;
    mutex_unlock(&(pcb->m));
    return 0;
}

int pcb_dec_children(pcb_t *pcb) {
    if (pcb == NULL) return -1;
    mutex_lock(&(pcb->m));
    pcb->num_child_proc--;
    mutex_unlock(&(pcb->m));
    return 0;
}

int pcb_inc_threads(pcb_t *pcb) {
    if (pcb == NULL) return -1;
    mutex_lock(&(pcb->m));
    pcb->num_threads++;
    mutex_unlock(&(pcb->m));
    return 0;
}

int pcb_dec_threads(pcb_t *pcb) {
    if (pcb == NULL) return -1;
    mutex_lock(&(pcb->m));
    pcb->num_threads--;
    mutex_unlock(&(pcb->m));
    return 0;
}

int pcb_get_child_count(pcb_t *pcb) {
    if (pcb == NULL) return -1;
    return pcb->num_child_proc;
}


