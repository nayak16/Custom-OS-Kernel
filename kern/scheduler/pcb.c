/** @file pcb_t.h
 *
 */

#include <stdlib.h>
#include <string.h>

#include <constants.h>
#include <pcb.h>

/* set_pdbr */
#include <special_reg_cntrl.h>

/* get_bytes */
#include <loader.h>
#include <mem_section.h>
#include <virtual_mem_manager.h>


/* PAGE_SIZE */
#include <x86/page.h>
#include <elf_410.h>
#include <loader.h>

#include <debug.h>
#include <simics.h>

/* TODO: Should this go here?? */
#define DIVROUNDUP(num, den) ((num + den-1) / den)

/* TODO: Should this go here?? */
#define NUM_ELF_SECTIONS 4


int pcb_init(pcb_t *pcb){
    if (pcb == NULL) return -1;
    /* Temp value before being added to scheduler */
    pcb->id = -1;

    pd_init(&(pcb->pd));
    return 0;
}

int pcb_set_running(pcb_t *pcb){
    if (pcb == NULL) return -1;
    /* set the current active page table to process's page table */
    return 0;
}

int pcb_destroy(pcb_t *pcb){
    panic("pcb_destroy: Not yet implemented");
    if (pcb == NULL) return -1;
    return 0;
}

int pcb_copy(pcb_t *dest_pcb, pcb_t *source_pcb) {
    if(source_pcb == NULL || dest_pcb == NULL) return -1;

    /* Copy address space */
    if(pd_copy(&(dest_pcb->pd), &(source_pcb->pd)) < 0) {
        return -3;
    }

    return 0;
}


int pcb_load_prog(pcb_t *pcb, const char *filename){
    if (pcb == NULL || filename == NULL) return -1;
    simple_elf_t elf;
    if (elf_check_header(filename) != ELF_SUCCESS) return -2;
    if (elf_load_helper(&elf, filename) != ELF_SUCCESS) return -3;

    /* Load elf sections into memory */
    if (load_elf_sections(&elf, pcb) < 0) return -4;

    /* Load user stack into memory */
    if (load_user_stack(pcb) < 0) return -5;

    /* TODO: Change this to be actual args */
    pcb->argc = 0;
    pcb->argv = NULL;

    return 0;
}
