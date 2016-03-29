/** @file special_reg_cntrl.h
 *  @brief Specifies w
 *
 *  @author Aatish Nayak (aatishn)
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs.
 */

#ifndef _SPECIAL_REG_CNTRL_H_
#define _SPECIAL_REG_CNTRL_H_

// TODO: Fill in with EFLAGS manipulation functions

void set_pdbr(uint32_t new_pdbr);
void set_cur_esp(uint32_t new_esp);

uint32_t get_pdbr();

void enable_paging(void);

void enable_pge(void);

uint32_t get_user_eflags(void);

void flush_tlb(uint32_t mem_addr);

#endif /* SPECIAL_REG_CNTRL_H_ */


