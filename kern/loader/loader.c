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
#include <x86/asm.h>

#include <constants.h>
#include <pcb.h>
#include <frame_manager.h>
#include <mem_section.h>
#include <special_reg_cntrl.h>

#include <simics.h>

#include <kern_internals.h>

#include <virtual_mem_mgmt.h>
#define NUM_ELF_SECTIONS 4


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

            /* Copy all size bytes if possible */
            int num_bytes = size + offset > entry.execlen
                ? entry.execlen - offset
                : size;

            memcpy(buf, entry.execbytes+offset, num_bytes);
            return num_bytes;
        }
    }
    return -1;
}

int get_all_files(char *buf, int buf_len) {
    if (buf == NULL) return -1;
    int i;
    int count_bytes = 0;
    for (i = 0; i < exec2obj_userapp_count; i++){
        exec2obj_userapp_TOC_entry entry = exec2obj_userapp_TOC[i];
        const char *filename = entry.execname;

        int filename_len = strlen(filename);
        lprintf("Filename: %s with len %d. count_bytes = %d",
                filename, filename_len, count_bytes);
        /* Check if buffer has anymore room */
        if (filename_len + count_bytes > buf_len) {
            return -2;
        }
        /* Copy in filename */
        memcpy(buf, filename, filename_len);

        /* Increment count_bytes and filename_len */
        buf += filename_len;
        count_bytes += filename_len;
    }
    return count_bytes;
}

int load_elf_exists(const char *filename){
    simple_elf_t elf;
    return (elf_check_header(filename) == ELF_SUCCESS &&
            elf_load_helper(&elf, filename) == ELF_SUCCESS);
}

int load_elf_sections(simple_elf_t *elf, pcb_t *pcb){
    mem_section_t secs[NUM_ELF_SECTIONS];

    ms_init(&secs[0], elf->e_txtstart,
                        elf->e_txtlen, USER_WR, USER_RO);
    ms_init(&secs[1], elf->e_datstart,
                        elf->e_datlen, USER_WR, USER_WR);
    ms_init(&secs[2], elf->e_rodatstart,
                        elf->e_rodatlen, USER_WR, USER_RO);
    ms_init(&secs[3], elf->e_bssstart,
                        elf->e_bsslen, USER_WR, USER_WR);

    /* Map all appropriate elf binary sections into user space */
    if (vmm_map_sections(&(pcb->pd), secs, NUM_ELF_SECTIONS) < 0) return -1;

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

    ms_init(&stack_secs[0], USER_STACK_BOTTOM,
                        USER_STACK_SIZE, USER_WR, USER_WR);

    /* Allocate and map space for stack in virtual memory */
    if (vmm_map_sections(&(pcb->pd), stack_secs, 1) < 0) return -2;

    /* Setup user stack for entry point */
    uint32_t *stack_top = (uint32_t *) USER_STACK_TOP;

    /* Loop through argv and allocate data onto user stack */
    char *arg;
    /* Will hold addresses of each arg string to be put into memory later */

    char *new_argv[pcb->argc];
    int arg_idx;
    char *esp = (char*)stack_top;
    for (arg_idx = 0; arg_idx < pcb->argc ; arg_idx++) {
        /* Get arg from provided argv */
        arg = pcb->argv[arg_idx];
        /* Calculate lenght of string arg */
        int arg_len = strlen(arg);
        /* Since we are filling in memory backwards, start with null char */
        *esp = '\0';

        int str_i = arg_len-1;
        /* Loop backwards through arg string and fill in esp */
        while(str_i >= 0) {
            esp--;
            *esp = arg[str_i];
            str_i--;
        }
        /* Save addr of start of string */
        new_argv[arg_idx] = esp;
        /* Make room for next arg */
        esp--;
    }

    /* Round esp down to nearest 4-byte boundary */
    stack_top = (uint32_t*)((uint32_t)esp - (uint32_t)((uint32_t)esp % 4));

    /* Go one space down to make room for first address */
    stack_top--;
    int i = pcb->argc - 1;
    /* Fill in addresses of strings into memory */
    while(i >= 0) {
        *stack_top = (uint32_t)new_argv[i];
        i--;
        stack_top--;
    }
    /* Save final argv value */
    uint32_t *final_argv = stack_top + 1;
    /* Specify dummy return address */
    stack_top[-4] = 0;
    /* argc for _main */
    stack_top[-3] = pcb->argc;
    /* argv for _main */
    stack_top[-2] = (uint32_t) final_argv;
    /* stack_high for _main */
    stack_top[-1] = USER_STACK_TOP;
    /* stack_low for _main */
    stack_top[0] = USER_STACK_BOTTOM;

    /* Set pcb stack */
    pcb->stack_top = (uint32_t) &(stack_top[-4]);

    return 0;
}

