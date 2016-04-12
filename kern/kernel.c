/** @file kernel.c
 *  @brief An initial kernel.c
 *
 *  You should initialize things in kernel_main(),
 *  and then run stuff.
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

/* Kernel global variables and internals */
#include <kern_internals.h>

/* Debugging */
#include <simics.h>                 /* lprintf() */
#include <debug.h>

#define KEYBOARD_BUFFER_SIZE 1024

/* Global vars */
scheduler_t sched;
mutex_t heap_lock;
frame_manager_t fm;
mutex_t console_lock;
keyboard_t keyboard;
sched_mutex_t sched_lock;

/** @brief Reaper entrypoint
 *
 *  @return Does not return
 */
#include <thr_helpers.h>
void reaper_main(){
    while(1){
        scheduler_reap(&sched);
    }
}


/** @brief Kernel entrypoint.
 *
 *  This is the entrypoint for the kernel.
 *
 * @return Does not return
 */
int kernel_main(mbinfo_t *mbinfo, int argc, char **argv, char **envp)
{
    lprintf("Shrek is love, Shrek is life");

    /* install IDT entries for system calls */
    install_syscall_handlers();

    /* install IDT entries for peripherals */
    install_peripheral_handlers();

    /* Install IDT entres for exceptions */
    install_exception_handlers();

    clear_console();

    /* Init global heap lock */
    mutex_init(&heap_lock);
    /* Init console mutex */
    mutex_init(&console_lock);

    /* initialize the keyboard buffer */
    keyboard_init(&keyboard, KEYBOARD_BUFFER_SIZE);
    /* init frame manager */
    fm_init(&fm, 15);
    /* initialize pd kernel pages */
    pd_init_kernel();

    /* Init the scheduler lock */
    sched_mutex_init(&sched_lock, &sched);

    /* initialize a scheduler */
    scheduler_init(&sched, reaper_main);

    scheduler_start(&sched); // enable intterupts

    while (1) {
        continue;
    }

    return 0;
}
