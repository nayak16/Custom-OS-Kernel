/** @file pcb_t.h
 *
 */

#include <stdlib.h>
#include <string.h>

#include <constants.h>
#include <pcb.h>
#include <queue.h>

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
    // TODO: set this to 0 and find good place to increment count
    pcb->num_threads = 1;
    /* Initialize an empty queue; use semaphore to allocate resources
     * when they become avaliable */
    queue_init(&(pcb->status_queue));
    sem_init(&(pcb->wait_sem), 0);
    pd_init(&(pcb->pd));
    return 0;
}

int pcb_destroy(pcb_t *pcb){
    if (pcb == NULL) return -1;
    sem_destroy(&(pcb->wait_sem));
    queue_destroy(&(pcb->status_queue));
    pd_destroy(&(pcb->pd));
    return 0;
}

int pcb_copy(pcb_t *dest_pcb, pcb_t *source_pcb) {
    if(source_pcb == NULL || dest_pcb == NULL) return -1;

    /* Set ppid of dest_pcb to pid of source_pcb */
    dest_pcb->ppid = source_pcb->pid;

    /* Copy address space */
    if(vmm_deep_copy(&(dest_pcb->pd)) < 0) {
        return -3;
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
    pcb_metadata_t *metadata = malloc(sizeof(pcb_metadata_t));
    metadata->status = status;
    metadata->original_tid = original_tid;
    if (queue_enq(&(pcb->status_queue), (void *) metadata) < 0) return -2;
    sem_signal(&(pcb->wait_sem));
    return 0;
}

int pcb_wait_on_status(pcb_t *pcb, int *status_ptr, int *original_pid){
    if (pcb == NULL) return -1;
    sem_wait(&(pcb->wait_sem));

    pcb_metadata_t *metadata;
    if (queue_deq(&(pcb->status_queue), (void **)&metadata) < 0)
        return -2;
    if (status_ptr != NULL) *status_ptr = metadata->status;
    if (original_pid != NULL) *original_pid = metadata->original_tid;

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

int pcb_add_child(pcb_t *pcb) {
    pcb->num_child_proc++;
    return 0;
}

int pcb_remove_child(pcb_t *pcb) {
    pcb->num_child_proc--;
    return 0;
}

int pcb_get_child_count(pcb_t *pcb) {
    return pcb->num_child_proc;
}

