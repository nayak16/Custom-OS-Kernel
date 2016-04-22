/** @file kernel.c
 *  @brief An initial kernel.c
 *
 *  Entry Point for ShrekOS
 *
 *  @author Aatish Nayak (aatishn)
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs.
 */

#include <common_kern.h>

/* libc includes. */
#include <stdio.h>

/* multiboot header file */
#include <multiboot.h>              /* boot_info */

/* x86 specific includes */
#include <x86/asm.h>                /* enable_interrupts() */

/* panic */
#include <stdlib.h>

/* console */
#include <console.h>

/* idt installers */
#include <install_handlers.h>
/* frame manager include */
#include <frame_manager.h>
/* process control block include */
#include <pcb.h>
/* control for special register wrapper */
#include <special_reg_cntrl.h>
#include <scheduler.h>
#include <mutex.h>
#include <queue.h>

/* smp includes */
#include <smp/mptable.h>
#include <smp/apic.h>
#include <smp/smp.h>

#include <lmm/lmm.h>
#include <malloc/malloc_internal.h>

/* Kernel global variables and internals */
#include <kern_internals.h>

/* Debugging */
#include <simics.h>                 /* lprintf() */

/**
 * @brief Max size of keyboard buffer
 * We arrived at this value because it is the same value provided to us
 * in the shell program
 */
#define KEYBOARD_BUFFER_SIZE 1024

/* Global vars */
scheduler_t sched;
scheduler_t *sched_arr;
mutex_t heap_lock;
frame_manager_t fm;
mutex_t console_lock;
keyboard_t keyboard;
sched_mutex_t sched_lock;

/** @brief Reaper entrypoint
 *
 *  @return Does not return
 */
// TODO: Change reaper_main to take in a scheduler argument not global
void reaper_main(){
    while(1){
        scheduler_reap(&sched);
    }
}


void ap_main(int cpu_num){
    lprintf("I am cpu %d", cpu_num);
    /* Spin until APIC is calibrated */
    while(!apic_calib_init_val);

    scheduler_init(&sched, reaper_main);

    /* Configure APIC timer vals */
    install_lapic_timer(apic_calib_init_val);

    /* Turn on APIC Timer */
    enable_interrupts();
    void* buf = malloc(sizeof(uint32_t) * PAGE_SIZE);
    lprintf("CPU %d allocated a buf starting at %p", smp_get_cpu(), buf);

    while(1);
}

/** @brief Kernel entrypoint.
 *
 *  This is the entrypoint for the kernel.
 *
 * @return Does not return
 */
int kernel_main(mbinfo_t *mbinfo, int argc, char **argv, char **envp)
{
    lprintf("Welcome to ShrekOS");
    lprintf("Shrek is love, Shrek is life");

    if (smp_init(mbinfo) < 0){
        panic("smp_init failed!");
    }

    int num_cores = smp_num_cpus();

    /* install IDT entries for system calls */
    install_syscall_handlers();

    /* install IDT entries for peripherals */
    install_legacy_peripheral_handlers();

    /* Install IDT entres for exceptions */
    install_exception_handlers();

    /* Clear the console */
    clear_console();

    /* Init global heap lock */
    mutex_init(&heap_lock);
    /* Init console mutex */
    mutex_init(&console_lock);

    /* initialize the keyboard buffer */
    keyboard_init(&keyboard, KEYBOARD_BUFFER_SIZE);

    /* init frame manager for core0 with num_cores cores*/
    fm_init(&fm, 15, 0, num_cores);

    /* initialize pd kernel pages */
    pd_init_kernel(num_cores);

    /* Malloc space for a scheduler per core */
    sched_arr = malloc(sizeof(scheduler_t) * num_cores);
    if (sched_arr == NULL) {
        panic("Not enough space to create a scheduler per core");
    }

    /* Init the scheduler lock */
    sched_mutex_init(&sched_lock, &sched);
    /* initialize a scheduler */
    scheduler_init(&sched, reaper_main);

    lprintf("Number of processors detected: %d", num_cores);
    /* Initialize mem regions of all available cores */
    unsigned long heap_space_left = lmm_avail(&malloc_lmm, 0);
    lprintf("Heap space remaining: %ld", (unsigned long) heap_space_left);
    if (num_cores > 1){
        unsigned long heap_size = heap_space_left/(num_cores-1);
        int core;
        for (core = 1; core < num_cores; core++){
            void *smidge = lmm_alloc(&malloc_lmm, heap_size, 0);
            lprintf("Allocating core %d %ld space starting at %p",
                    core, heap_size, smidge);
            if (smidge == NULL) panic("oopsies");
            lmm_add_free(&core_malloc_lmm[core], smidge, heap_size);
        }
    }
    install_lapic_timer(-1);
    smp_boot(ap_main);
    //scheduler_start(&sched);
    enable_interrupts();
    while (1) {
        continue;
    }

    return 0;
}
