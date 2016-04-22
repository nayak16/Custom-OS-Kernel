/** @file install_handlers.h
 *  @brief specifications for handler install functions
 *
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)
 *  @bug No known bugs.
 */

#ifndef _INSTALL_HANDLERS_H_
#define _INSTALL_HANDLERS_H_


int install_syscall_handlers(void);

int install_legacy_peripheral_handlers(void);

int install_lapic_timer(uint32_t init_val);

int install_exception_handlers(void);

#endif /* _INSTALL_HANDLERS_H_ */
