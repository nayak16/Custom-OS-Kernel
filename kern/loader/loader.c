/**
 * The 15-410 kernel project.
 * @name loader.c
 *
 */

/* --- Includes --- */
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <exec2obj.h>
#include <loader.h>
#include <elf_410.h>

#include <constants.h>
#include <pcb.h>
#include <frame_manager.h>
#include <virtual_mem_manager.h>
#include <mem_section.h>

#include <simics.h>

#include <kern_internals.h>
#define NUM_ELF_SECTIONS 4

#define USER_RO NEW_FLAGS(SET, UNSET, SET, UNSET)
#define USER_WR NEW_FLAGS(SET, SET, SET, UNSET)


/* --- Local function prototypes --- */


/**
 * Copies data from a file into a buffer.
 *
 * @param filename   the name of the file to copy data from
 * @param offset     the location in the file to begin copying from
 * @param size       the number of bytes to be copied
 * @param buf        the buffer to copy the data into
 *
 * @return returns the number of bytes copied on succes; -1 on failure
 */
int getbytes( const char *filename, int offset, int size, char *buf )
{
    int i;
    for (i = 0; i < exec2obj_userapp_count; i++){
        exec2obj_userapp_TOC_entry entry = exec2obj_userapp_TOC[i];

        if (strcmp(entry.execname, filename) == 0){
            int execlen = entry.execlen;
            /* check if we have enough bytes  */
            if (size + offset > execlen){
                return -2;
            }
            memcpy(buf, entry.execbytes+offset, size);
            return size;
        }
    }
    return -1;
}

int load_elf_sections(simple_elf_t *elf, pcb_t *pcb){
    mem_section_t secs[NUM_ELF_SECTIONS];

    mem_section_init(&secs[0], elf->e_txtstart,
                        elf->e_txtlen, NULL, USER_WR, USER_RO);
    mem_section_init(&secs[1], elf->e_datstart,
                        elf->e_datlen, NULL, USER_WR, USER_WR);
    mem_section_init(&secs[2], elf->e_rodatstart,
                        elf->e_rodatlen, NULL, USER_WR, USER_RO);
    mem_section_init(&secs[3], elf->e_bssstart,
                        elf->e_bsslen, NULL, USER_WR, USER_WR);

    /* Map all appropriate elf binary sections into user space */
    if (vmm_mem_alloc(&(pcb->pd), secs, NUM_ELF_SECTIONS) < 0) return -1;

    /* Fill in .text section */
    if (getbytes(elf->e_fname, elf->e_txtoff, elf->e_txtlen,
                 (char*)elf->e_txtstart) != elf->e_txtlen) return -1;

    /* Fill in .data section */
    if (getbytes(elf->e_fname, elf->e_datoff, elf->e_datlen,
                 (char*)elf->e_datstart) != elf->e_datlen) return -1;

    /* Fill in .rodata section */
    if (getbytes(elf->e_fname, elf->e_rodatoff, elf->e_rodatlen,
                 (char*)elf->e_rodatstart) != elf->e_rodatlen) return -1;

    /* Clear .bss section */
    memset((void *)elf->e_bssstart, 0, elf->e_bsslen);

    /* Set process entry point */
    pcb->entry_point = elf->e_entry;
    return 0;
}


int load_user_stack(pcb_t *pcb) {
    if (pcb == NULL) return -1;

    /* Calculate user stack bottom */
    mem_section_t stack_secs[1];

    mem_section_init(&stack_secs[0], USER_STACK_BOTTOM,
                        USER_STACK_SIZE, NULL, USER_WR, USER_WR);

    /* Allocate and map space for stack in virtual memory */
    if (vmm_mem_alloc(&(pcb->pd), stack_secs, 1) < 0) return -2;

    /* Setup user stack for entry point */
    uint32_t *stack_top = (uint32_t *) USER_STACK_TOP;
    stack_top[-3] = pcb->argc;
    stack_top[-2] = (uint32_t) pcb->argv;
    stack_top[-1] = USER_STACK_TOP;
    stack_top[0] = USER_STACK_BOTTOM;

    /* Specify dummy return address */
    stack_top[-4] = 0;

    /* Set pcb stack */
    pcb->stack_top = (uint32_t) &(stack_top[-4]);

    return 0;
}

