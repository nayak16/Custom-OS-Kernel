/** @file idt_handlers.h
 *  @brief specifies handlers for system exceptions
 *
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)
 *  @bug No known bugs.
 */
#ifndef _IDT_HANDLERS_H_
#define _IDT_HANDLERS_H_

#include <ureg.h>

/* Exception Handlers */

void page_fault_handler(void);

void double_fault_handler(void);

// TODO: Add more

/* Syscall thread management handlers */

int syscall_gettid_handler(void);
int syscall_yield_handler(int);
int syscall_deschedule_handler(int *);
int syscall_make_runnable_handler(int);
int syscall_get_ticks_handler(void);
int syscall_sleep_handler(int);

int syscall_swexn_handler(void *, void (*)(void *, ureg_t *), void *, ureg_t *);

/* Syscall life cycle handlers */

int syscall_fork_handler(void);
void syscall_vanish_handler(void);
int syscall_wait_handler(int *status_ptr);
void syscall_set_status_handler(int);
int syscall_exec_handler(char *execname, char **argvec);

/* Console IO handlers */

int syscall_readline_handler(int len, char *buf);
int syscall_print_handler(int len, char *buf);
int syscall_set_term_color_handler(int color);
int syscall_set_cursor_pos_handler(int row, int col);
int syscall_get_cursor_pos_handler(int *row, int *col);

/* Mem mgmt handlers*/

int syscall_new_pages_handler(void *base, int len);
int syscall_remove_pages_handler(void *base);

/* Misc handlers */

void syscall_halt_handler(void);

/* Hardware handlers */

void timer_handler(void);

void keyboard_handler(uint32_t old_esp);

#endif /* IDT_HANDLERS_H_ */
