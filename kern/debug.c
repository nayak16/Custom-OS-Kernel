/** @file debug.h
 *  @brief Implementation of debugging functions
 *
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)
 *  @bug No known bugs.
 */
#include <x86/cr.h>

#include <elf_410.h>

#include <page_directory.h>
#include <constants.h>
#include <tcb.h>

#include <simics.h>

void print_control_regs(void) {
    lprintf("----- Control Registers -----");
    lprintf("cr0 0x%x", (unsigned int) get_cr0());
    lprintf("cr2 0x%x", (unsigned int) get_cr2());
    lprintf("cr3 0x%x", (unsigned int) get_cr3());
    lprintf("cr4 0x%x", (unsigned int) get_cr4());

}

void print_context(unsigned int *stack) {
    lprintf("Stack: %p", stack);
    lprintf("------- Context --------");
    lprintf("ss: 0x%x", stack[SS_IDX]);
    lprintf("esp: 0x%x", stack[ESP_IDX]);
    lprintf("eflags: 0x%x", stack[EFLAGS_IDX]);
    lprintf("cs: 0x%x", stack[CS_IDX]);
    lprintf("eip: 0x%x", stack[EIP_IDX]);
    lprintf("ecx: 0x%x", stack[ECX_IDX]);
    lprintf("edx: 0x%x", stack[EDX_IDX]);
    lprintf("ebx: 0x%x", stack[EBX_IDX]);
    lprintf("ebp: 0x%x", stack[EBP_IDX]);
    lprintf("esi: 0x%x", stack[ESI_IDX]);
    lprintf("edi: 0x%x", stack[EDI_IDX]);
    lprintf("ds: 0x%x", stack[DS_IDX]);
    lprintf("es: 0x%x", stack[ES_IDX]);
    lprintf("fs: 0x%x", stack[FS_IDX]);
    lprintf("gs: 0x%x", stack[GS_IDX]);
    lprintf("------ End Context -------");
}

void print_page_directory(page_directory_t *pd, int start, int len, int verbose){
    uint32_t *directory = pd->directory;
    lprintf("----- Page Directory -----");
    lprintf("Page Directory Base Address: %p", directory);
    int i, j;
    for (i = start+len-1; i >= start; i--){
        if ((directory[i] & 1) != 0){
            lprintf("PDE #%d <%p>", i, (void *)directory[i]);
            unsigned int *temp =
                (unsigned int *)((unsigned int)directory[i] & MSB_20_MASK);
            if (verbose){
                for (j = 1023; j >= 0; j--){
                    lprintf("> PTE #%d : 0x%x", j, temp[j]);
                }
            } else {
                lprintf("> PTE #1023 : 0x%x", temp[1023]);
                lprintf("> PTE #1022 : 0x%x", temp[1022]);
                lprintf("> PTE #1021 : 0x%x", temp[1021]);
                lprintf("> PTE #1020 : 0x%x", temp[1020]);
                lprintf("> PTE #1019 : 0x%x", temp[1019]);
                lprintf("> PTE #1018 : 0x%x", temp[1018]);
                lprintf("> ....");
                lprintf("> PTE #0002 : 0x%x", temp[2]);
                lprintf("> PTE #0001 : 0x%x", temp[1]);
                lprintf("> PTE #0000 : 0x%x", temp[0]);
            }
        }
    }
}

void translate_addr(page_directory_t *pd, unsigned int addr) {
    unsigned int *dir = (unsigned int*)pd->directory;
    lprintf("---- Translating Virtual Addr %d ----", addr);
    unsigned int top_10 = addr & 0xffc00000;
    unsigned int middle_10 = addr & 0x003ff000;
    unsigned int pde = top_10 / PAGE_SIZE;
    unsigned int pte = middle_10 % 1024; //NUM_ENTRIES removed
    lprintf("PDE: 0x%x", pde);
    lprintf("PTE: 0x%x", pte);
    if ((dir[pte] & 1) == 0) {
        lprintf("PDE NOT PRESENT");
        return;
    } else {
        lprintf("pte:%d %p", pte, (void *)dir[pte]);
            unsigned int *temp =
                (unsigned int *)((unsigned int)dir[pte] & MSB_20_MASK);
        lprintf("-->[%x]", temp[1023]);
            lprintf("-->[%x]", temp[1022]);
            lprintf("   ....");
            lprintf("-->[%x]", temp[2]);
            lprintf("-->[%x]", temp[1]);
            lprintf("-->[%x]", temp[0]);


    }

}

void print_elf(simple_elf_t *elf) {
    lprintf("---- Elf containing '%s' ----", elf->e_fname);
    lprintf("Text Start: 0x%x Len: 0x%x", (unsigned int) elf->e_txtstart, (unsigned int) elf->e_txtlen);
    lprintf("Data Start: 0x%x Len: 0x%x", (unsigned int) elf->e_datstart, (unsigned int) elf->e_datlen);
    lprintf("BSS Start: 0x%x Len: 0x%x", (unsigned int) elf->e_bssstart, (unsigned int) elf->e_bsslen);

}

