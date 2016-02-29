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

void print_page_directory(page_directory_t *pd);

void print_control_regs(void);

#endif /* DEBUG_H_ */
