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
#include <elf/elf_410.h>
#include <elf_410.h>

/* TODO: Should this go here?? */
#define DIVROUNDUP(num, den) ((num + den-1) / den)

/* TODO: Should this go here?? */
#define NUM_ELF_SECTIONS 4

#define USER_RO NEW_FLAGS(SET, UNSET, SET, UNSET)
#define USER_WR NEW_FLAGS(SET, SET, SET, UNSET)

int load_sections(simple_elf_t *elf, pcb_t *pcb, frame_manager_t *fm){
    char text_sec[elf->e_txtlen];
    char dat_sec[elf->e_datlen];
    char rodat_sec[elf->e_rodatlen];
    char bss_sec[elf->e_bsslen];
    /* Set bss contents to 0 */
    memset(bss_sec, 0, sizeof(bss_sec));

    int bytes;
    if ((bytes = getbytes(elf->e_fname, elf->e_txtoff,
                            sizeof(text_sec), text_sec)) != sizeof(text_sec))
                                                                return -1;
    if ((bytes = getbytes(elf->e_fname, elf->e_datoff,
                            sizeof(dat_sec), dat_sec)) != sizeof(dat_sec))
                                                                return -1;
    if ((bytes = getbytes(elf->e_fname, elf->e_rodatoff,
                            sizeof(rodat_sec), rodat_sec)) != sizeof(rodat_sec))
                                                                return -1;

    mem_section_t secs[NUM_ELF_SECTIONS];

    mem_section_init(&secs[0], elf->e_txtstart,
                     elf->e_txtlen, (void*) text_sec, USER_RO, USER_RO);

    mem_section_init(&secs[1], elf->e_datstart,
                     elf->e_datlen, (void*) dat_sec, USER_WR, USER_WR);

    mem_section_init(&secs[2], elf->e_rodatstart,
                     elf->e_rodatlen, (void*) rodat_sec, USER_RO, USER_RO);

    mem_section_init(&secs[3], elf->e_bssstart,
                     elf->e_bsslen, (void *) bss_sec, USER_WR, USER_WR);

    if (vmm_user_mem_alloc(&(pcb->pd), fm, secs, NUM_ELF_SECTIONS) < 0) return -1;

    // TODO: load section from buf into physical memory, map physical memory to
    // virtual memory, allocate physical memory from fma
    return 0;
}

int pcb_init(pcb_t *pcb){
    if (pcb == NULL) return -1;
    //TODO: how to choose pid
    pcb->pid = 1;
    pd_init(&(pcb->pd));
    pd_initialize_kernel(&(pcb->pd));
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

int pcb_load(pcb_t *pcb, frame_manager_t *fm, const char *filename){
    if (pcb == NULL || filename == NULL || fm == NULL) return -1;
    simple_elf_t elf;
    if (elf_check_header(filename) != ELF_SUCCESS) return -1;
    if (elf_load_helper(&elf, filename) != ELF_SUCCESS) return -1;
    /* for each section, load the appropriate section */
    if (load_sections(&elf, pcb, fm) < 0) return -1;
    // set first tcb
    return -1;
}
