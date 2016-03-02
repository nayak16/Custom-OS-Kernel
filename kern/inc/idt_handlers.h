/** @file idt_handlers.h
 *  @brief specifies handlers for system exceptions
 *
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)
 *  @bug No known bugs.
 */
#ifndef _IDT_HANDLERS_H_
#define _IDT_HANDLERS_H_

/* Exception Handlers */

void page_fault_handler(void);

void double_fault_handler(void);

// TODO: Add more

/* Syscall handlers */

int syscall_gettid_handler(void);

/* Hardware handlers */

void timer_handler(void);

#endif /* IDT_HANDLERS_H_ */
