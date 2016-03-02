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


/* Debugging */
#include <simics.h>                 /* lprintf() */
#include <debug.h>

/** @brief Kernel entrypoint.
 *
 *  This is the entrypoint for the kernel.
 *
 * @return Does not return
 */
int kernel_main(mbinfo_t *mbinfo, int argc, char **argv, char **envp)
{
    lprintf("Hello from ShrekOS");

    /* install IDT entries for system calls */
    install_syscall_handlers();

    /* install IDT entries for peripherals */
    install_peripheral_handlers();

    /* Install IDT entres for exceptions */
    install_exception_handlers();

    clear_console();

    /* create frame manager */
    frame_manager_t fm;
    fm_init(&fm);

    /* initialize idle_pcb */
    pcb_t idle_pcb;
    pcb_init(&idle_pcb);

    /* initialize a scheduler */
    scheduler_t sched;
    scheduler_init(&sched);

    /* setup first page table so paging works in pcb_load */
    set_pdbr((uint32_t) pd_get_base_addr(&idle_pcb.pd));
    /* Enable Page Global Flag */
    enable_pge();
    /* Enable Paging */
    enable_paging();

    /* load idle program */
    pcb_load_prog(&idle_pcb, &fm, "idle");

    /* add idle process to scheduler */
    scheduler_add_process(&sched, &idle_pcb);

    // TODO: create and load init task

    // TODO: set first thread running

    //scheduler_start(&sched); // enable intterupts


    int i = 0;
    while (1) {
        i++;
        continue;
    }

    return 0;
}
