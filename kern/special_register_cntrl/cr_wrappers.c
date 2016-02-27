/** @file special_reg_cntrl.h
 *  @brief Specifies w
 *
 *  @author Aatish Nayak (aatishn)
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs.
 */

#include <stdint.h>

#include <x86/cr.h>

#include <constants.h>

#define ENABLE_PAGING_BIT 31

#define DISABLE_CACHING_BIT 30

#define PGE_FLAG_BIT 7

/**
 * @brief Sets top 20 bit of cr3 to page directory base address
 *
 * @param address of new page directory (should be page-aligned)
 */
void set_pdbr(uint32_t new_pdbr) {
    uint32_t empty_cr3 = get_cr3() & LSB_12_MASK;
    uint32_t new_cr3 = new_pdbr | empty_cr3;

    set_cr3(new_cr3);
}

void enable_paging(void) {
    uint32_t new_cr0 = (SET << ENABLE_PAGING_BIT) | (SET << DISABLE_CACHING_BIT);

    set_cr0(new_cr0);
}

void enable_pge(void) {
    uint32_t new_cr4 = get_cr4() | (SET << PGE_FLAG_BIT);

    set_cr4(new_cr4);
}

