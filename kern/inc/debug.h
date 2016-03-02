/** @file debug.h
 *  @brief specifies debug routines for the kernel
 *
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)
 *  @bug No known bugs.
 */
#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <page_directory.h>
#include <elf/elf_410.h>

void print_page_directory(page_directory_t *pd, int s, int l);

void print_control_regs(void);
void translate_addr(page_directory_t *pd, uint32_t addr);
void print_elf(simple_elf_t *elf);

#endif /* DEBUG_H_ */
