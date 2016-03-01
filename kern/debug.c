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

#include <simics.h>

void print_control_regs(void) {
    lprintf("----- Control Registers -----");
    lprintf("cr0 0x%x", (unsigned int) get_cr0());
    lprintf("cr2 0x%x", (unsigned int) get_cr2());
    lprintf("cr3 0x%x", (unsigned int) get_cr3());
    lprintf("cr4 0x%x", (unsigned int) get_cr4());

}

void print_page_directory(page_directory_t *pd){
    uint32_t *directory = pd->directory;
    lprintf("Page Directory Base Address: %p", directory);
    int i;
    for (i = 5; i >= 0; i--){
        if ((directory[i] & 1) == 0){
            lprintf("i:%d NOT PRESENT", i);
        } else {
            lprintf("i:%d %p", i, (void *)directory[i]);
            unsigned int *temp =
                (unsigned int *)((unsigned int)directory[i] & MSB_20_MASK);
            lprintf("-->[%x]", temp[1023]);
            lprintf("-->[%x]", temp[1022]);
            lprintf("   ....");
            lprintf("-->[%x]", temp[2]);
            lprintf("-->[%x]", temp[1]);
            lprintf("-->[%x]", temp[0]);
        }
    }
}

void print_elf(simple_elf_t *elf) {
    lprintf("---- Elf containing '%s' ----", elf->e_fname);
    lprintf("Text section starts at %x", (unsigned int) elf->e_txtstart);
    lprintf("Data section starts at %x", (unsigned int) elf->e_datstart);
    lprintf("Bss section starts at %x", (unsigned int) elf->e_bssstart);

}

