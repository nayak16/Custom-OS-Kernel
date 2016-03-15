/** @file
 *
 */


#ifndef _DISPATCHER_H_
#define _DISPATCHER_H_

#include <tcb.h>

void initial_mode_switch(tcb_t *tcb);
void initial_mode_switch_asm(tcb_t tcb);
void save_context(tcb_t *tcb);
void restore_context(tcb_t *tcb);


#endif /* _DISPATCHER_H_ */

