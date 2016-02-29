/** @file tcb_t.h
 *
 */

#ifndef _TCB_T_H_
#define _TCB_T_H_

typedef struct tcb{
    int tid;
    int status;
    int exit_status;
    // TODO: add register context
} tcb_t;

#endif /* _TCB_T_H_ */
