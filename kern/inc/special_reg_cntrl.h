/** @file special_reg_cntrl.h
 *  @brief Specifies w
 *
 *  @author Aatish Nayak (aatishn)
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs.
 */

#ifndef _SPECIAL_REG_CNTRL_H_
#define _SPECIAL_REG_CNTRL_H_

void set_pdbr(uint32_t new_pdbr);

/** @brief sets the current esp to the new_esp
 *  @param new_esp the new esp to set to
 *  @return Void
 */
void set_cur_esp(uint32_t new_esp);

uint32_t get_pdbr();

void enable_paging(void);

void enable_pge(void);

uint32_t get_user_eflags(void);

/** @brief flushes tlb containing address
 *  @param mem_addr The memory address to flush from
 *  @return Void */
void flush_tlb(uint32_t mem_addr);
void flush_all_tlb(void);

#endif /* SPECIAL_REG_CNTRL_H_ */


