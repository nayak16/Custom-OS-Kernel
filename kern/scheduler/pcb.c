/** @file pcb.h
 *
 *  @brief Implementation of process control block functions
 *
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


/**
 * @brief Initializes a process control block
 *
 * @param pcb pcb to initialize
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int pcb_init(pcb_t *pcb){
    if (pcb == NULL) return -1;

    /* Temp value before being added to scheduler */
    pcb->pid = -1;
    pcb->ppid = -1;
    pcb->original_tid = -1;
    pcb->num_child_proc = 0;
    pcb->num_threads = 0;

    /* Initialize a pcb's page directory */
    pd_init(&(pcb->pd));

    /* Init the mutex that protects pcb access */
    mutex_init(&(pcb->m));
    /* Initialize an empty queue; use semaphore to allocate resources
     * when they become avaliable */
    queue_init(&(pcb->status_queue));
    sem_init(&(pcb->wait_sem), 0);
    return 0;
}

/**
 * @brief Destroys a pcb and its data structures
 *
 * @param pcb pcb to destroy
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int pcb_destroy_s(pcb_t *pcb){
    if (pcb == NULL) return -1;

    /* Grab pcb lock */
    mutex_lock(&(pcb->m));

    /* Destroy the wait semaphore */
    sem_destroy(&(pcb->wait_sem));

    /* Destroy the status queue */
    queue_destroy(&(pcb->status_queue));

    /* Clears user space mappings in the page directory
     * This will also deallocate physical frames */
    vmm_clear_user_space(&(pcb->pd));

    /* Destroy page directory */
    pd_destroy(&(pcb->pd));

    mutex_unlock(&(pcb->m));
    mutex_destroy(&(pcb->m));

    return 0;
}

/**
 * @brief Creates a copy of a given pcb
 *
 * This is used in the fork() system call to duplicate
 * the address space for a process
 *
 * @param dest_pcb pcb to copy into
 * @param source_pcb pcb to copy from
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int pcb_copy(pcb_t *dest_pcb, pcb_t *source_pcb) {
    if(source_pcb == NULL || dest_pcb == NULL) return -1;

    /* Set parent pid of dest_pcb to pid of source_pcb
     * since source_pcb is the parent */
    dest_pcb->ppid = source_pcb->pid;

    /* Copy current user address space */
    if(vmm_deep_copy(&(dest_pcb->pd)) < 0) {
        return -2;
    }

    return 0;
}

/**
 * @brief Loads the specified program with the specified args into the
 * specified pcb. Also, loads the user level stack into the pcb's address
 * space.
 *
 * @param pcb pcb to load into
 * @param filename filename of elf executable. Returns error if filename is not
 * a valid elf file
 * @param argc number of arguments to _main of new program
 * @param argv array of string args to _main of new program
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int pcb_load_prog(pcb_t *pcb, const char *filename, int argc, char** argv){
    if (pcb == NULL || filename == NULL) return -1;
    simple_elf_t elf;

    /* Check if the filename is a valid ELF */
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

/**
 * @brief Struct to hold metadata about a tcbs's exit status and original tid.
 * This struct is placed in a parent pcb's status queue and is dequeued when a
 * parent pcb calls wait().
 *
 */
typedef struct pcb_metadata{
    int status;
    int original_tid;
} pcb_metadata_t;

/**
 * @brief Enqueues a pcb_metadata struct to the specified pcb's status queue.
 * Then signals the pcb's wait semaphore to indicate there is new data in the
 * status queue available.
 *
 * @param pcb pcb to signal
 * @param status exit status of the vanishing thread
 * @param original_tid original tid of the original thread of the child pcb
 * signaling the parent
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int pcb_signal_status(pcb_t *pcb, int status, int original_tid){
    if (pcb == NULL) return -1;

    /* Create struct to hold meta data */
    pcb_metadata_t *metadata = malloc(sizeof(pcb_metadata_t));
    if (metadata == NULL) {
        return -2;
    }
    metadata->status = status;
    metadata->original_tid = original_tid;

    /* Put status to collect into queue */
    if (queue_enq(&(pcb->status_queue), (void *) metadata) < 0) {
        mutex_unlock(&(pcb->m));
        return -2;
    }

    /* Signal that a status is available */
    sem_signal(&(pcb->wait_sem));

    return 0;
}

/**
 * @brief Waits on the wait sem of the specified pcb and collects the status
 * and original tid of the vanished child in the status queue.
 *
 * @param pcb pcb that will wait on its children. If this is the init pcb, it
 * will wait on any descendant children.
 * @param status_ptr address to put the collected status
 * @param original_tidp address to put the collected original tid
 *
 * @return 0 on success, negative error code otherwise
 */
int pcb_wait_on_status(pcb_t *pcb, int *status_ptr, int *original_tidp){
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
    if (original_tidp != NULL) *original_tidp = metadata->original_tid;

    mutex_unlock(&(pcb->m));
    /* Free metadata struct */
    free(metadata);
    return 0;
}

/**
 * @brief Gets the pid of the parent pcb
 *
 * @param pcb pcb to access
 *
 * @return pid of parent pcb, negative error code otherwise
 *
 */
int pcb_get_ppid(pcb_t *pcb){
    if (pcb == NULL) return -1;
    return pcb->ppid;
}

/**
 * @brief Sets the original tid of the specified pcb
 *
 * @param pcb pcb to set original tid of
 * @param tid original tid to set the property to
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int pcb_set_original_tid(pcb_t *pcb, int tid){
    if (pcb == NULL) return -1;
    pcb->original_tid = tid;
    return 0;
}

/**
 * @brief Gets the original tid of the specified pcb
 *
 * @param pcb pcb to get original tid of
 * @param tidp address to put the original tid
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int pcb_get_original_tid(pcb_t *pcb, int *tidp){
    if (pcb == NULL || tidp == NULL) return -1;
    *tidp = pcb->original_tid;
    return 0;
}

/**
 * @brief Safely increments the children of the specified pcb
 *
 * @param pcb pcb to increment count
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int pcb_inc_children_s(pcb_t *pcb) {
    if (pcb == NULL) return -1;
    mutex_lock(&(pcb->m));
    pcb->num_child_proc++;
    mutex_unlock(&(pcb->m));
    return 0;
}

/**
 * @brief Safely decrements the children of the specified pcb
 *
 * @param pcb pcb to decrement count
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int pcb_dec_children_s(pcb_t *pcb) {
    if (pcb == NULL) return -1;
    mutex_lock(&(pcb->m));
    pcb->num_child_proc--;
    mutex_unlock(&(pcb->m));
    return 0;
}

/**
 * @brief Safely increments the number of threads of the specified pcb
 *
 * @param pcb pcb to increment number of threads
 *
 * @return number of threads on success, negative error code otherwise
 *
 */
int pcb_inc_threads_s(pcb_t *pcb) {
    if (pcb == NULL) return -1;
    mutex_lock(&(pcb->m));
    pcb->num_threads++;
    int c = pcb->num_threads;
    mutex_unlock(&(pcb->m));
    return c;
}

/**
 * @brief Safely decrements the number of threads of the specified pcb
 *
 * @param pcb pcb to increment number of threads
 *
 * @return number of threads on success, negative error code otherwise
 *
 */
int pcb_dec_threads_s(pcb_t *pcb) {
    if (pcb == NULL) return -1;
    mutex_lock(&(pcb->m));
    pcb->num_threads--;
    int c = pcb->num_threads;
    mutex_unlock(&(pcb->m));
    return c;
}


