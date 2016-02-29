/** @file pcb_t.h
 *
 */

#include <stdlib.h>

#include <pcb.h>

/* set_pdbr */
#include <special_reg_cntrl.h>

/* get_bytes */
#include <loader.h>

/* PAGE_SIZE */
#include <page.h>

#include <elf_410.h>

#define DIVROUNDUP(num, den) ((num + den-1) / den)

/** @brief copies size bytes from f_src to memory and maps v_dest
 *
 */
int load_and_map_section(unsigned long f_src, unsigned long v_dest,
    unsigned long size, pcb_t *pcb, fm_t *fm, const char *filename){
    
    char buf[size];
    int bytes;
    if ((bytes = getbytes(filename, f_src, size, buf)) != size) return -1;
    int npages = DIVROUNDUP(bytes, PAGE_SIZE);
    int p;
    int size_remaining = size;
    for (p = 0; p < npages; p++){
        void **phys_addr;
        pd_mmap(*phys_addr, (void *)(v_dest+(p*PAGE_SIZE))
    }
}

int load_sections(elf_t *elf, pcb_t *pcb, fm_t *fm, const char *filename){
    char buf[size];
    int bytes;
    if ((bytes = getbytes(filename, f_src, size, buf)) != size) return -1;
    section_t sections[4] = {
        {.start=elf->e_txtstart, .len = elf->e_txtlen, .src=buf+elf->e_txtoffset},
        {.start=elf->e_datstart, //TODO FILL REST OF IN}
    };
    if (pd_map_load(&(pcb->pd), fm, sections) < 0) return -1;

    // TODO: load section from buf into physical memory, map physical memory to
    // virtual memory, allocate physical memory from fm
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
    set_pdbr((uint32_t)pd_get_base_addr(&(pcb->pd)));
    return 0;
}

int pcb_destroy(pcb_t *pcb){
    panic("pcb_destroy: Not yet implemented");
    if (pcb == NULL) return -1;
    return 0;
}

int pcb_load(pcb_t *pcb, fm_t *fm, const char *filename){
    if (pcb == NULL || filename == NULL || fm == NULL) return -1;
    simple_elf_t elf;
    if (elf_check_header(filename) != ELF_SUCCESS) return -1;
    if (elf_load_helper(&elf, filename) != ELF_SUCCESS) return -1;
    /* for each section, load the appropriate section */
    if (load_sections(&elf, pcb, fm, filename) < 0) return -1;
    // set first tcb
    MAGIC_BREAK;
    return -1;
}