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

/** @brief assembly wrapper for page fault handler */
void page_fault_handler(void);
/** @brief assembly wrapper for double fault handler */
void double_fault_handler(void);
/** @brief assembly wrapper for division error handler */
void division_error_handler(void);
/** @brief assembly wrapper for debug exception handler */
void debug_exception_handler(void);
/** @brief assembly wrapper for breakpoint handler */
void breakpoint_handler(void);
/** @brief assembly wrapper for overflow handler */
void overflow_handler(void);
/** @brief assembly wrapper for bound range handler */
void bound_range_handler(void);
/** @brief assembly wrapper for undef op handler */
void undef_op_handler(void);
/** @brief assembly wrapper for no math handler */
void no_math_handler(void);
/** @brief assembly wrapper for cso handler */
void coprocessor_segment_overrun_handler(void);
/** @brief assembly wrapper for invalid tss handler */
void invalid_tss_handler(void);
/** @brief assembly wrapper for segment not present handler */
void segment_not_present_handler(void);
/** @brief assembly wrapper for gp fault handler */
void gp_fault_handler(void);
/** @brief assembly wrapper for math fault handler */
void math_fault_handler(void);
/** @brief assembly wrapper for align fault handler */
void align_fault_handler(void);
/** @brief assembly wrapper for machine check fault handler */
void machine_check_fault_handler(void);
/** @brief assembly wrapper for simd fault handler */
void simd_fault_handler(void);


/* Syscall thread management handlers */
/** @brief syscall wrapper for gettid */
int syscall_gettid_handler(void);
/** @brief syscall wrapper for yield */
int syscall_yield_handler(int);
/** @brief syscall wrapper for deschedule */
int syscall_deschedule_handler(int *);
/** @brief syscall wrapper for make runnable*/
int syscall_make_runnable_handler(int);
/** @brief syscall wrapper for get ticks*/
int syscall_get_ticks_handler(void);
/** @brief syscall wrapper for sleep */
int syscall_sleep_handler(int);
/** @brief syscall wrapper for swexn */
int syscall_swexn_handler(void *, void (*)(void *, ureg_t *), void *, ureg_t *);

/* Syscall life cycle handlers */

/** @brief syscall wrapper for fork*/
int syscall_fork_handler(void);
/** @brief syscall wrapper for vanish*/
void syscall_vanish_handler(void);
/** @brief syscall wrapper for wait*/
int syscall_wait_handler(int *status_ptr);
/** @brief syscall wrapper for set status */
void syscall_set_status_handler(int);
/** @brief syscall wrapper for exec */
int syscall_exec_handler(char *execname, char **argvec);
/** @brief syscall wrapper for thread fork */
int syscall_thread_fork_handler(void);

/* Console IO handlers */

/** @brief syscall wrapper for readline */
int syscall_readline_handler(int len, char *buf);
/** @brief syscall wrapper for print */
int syscall_print_handler(int len, char *buf);
/** @brief syscall wrapper for set term color */
int syscall_set_term_color_handler(int color);
/** @brief syscall wrapper for set cursor pos */
int syscall_set_cursor_pos_handler(int row, int col);
/** @brief syscall wrapper for get cursor pos */
int syscall_get_cursor_pos_handler(int *row, int *col);

/* Mem mgmt handlers*/

/** @brief syscall wrapper for new pages */
int syscall_new_pages_handler(void *base, int len);
/** @brief syscall wrapper for remove pages */
int syscall_remove_pages_handler(void *base);

/* Misc handlers */

/** @brief syscall wrapper for halt */
void syscall_halt_handler(void);
/** @brief syscall wrapper for readfile */
int syscall_readfile_handler(char *filename, char *buf, int count, int offset);
/** @brief syscall wrapper for misbehave */
void syscall_misbehave_handler(int mode);

/* Hardware handlers */

/** @brief peripheral wrapper for the PIC timer */
void pit_timer_handler(void);
/** @brief peripheral wrapper for the LAPIC timer */
void lapic_timer_handler(void);
/** @brief peripheral wrapper for keyboard */
void keyboard_handler(void);

#endif /* IDT_HANDLERS_H_ */
