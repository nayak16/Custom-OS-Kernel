/** @file syscall_task_vanish.S
 *
 *  @brief implements task_vanish stub
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug
 */

#include <syscall_int.h>

.globl task_vanish

task_vanish:
    push %esi           /* save context */
    mov 8(%esp), %esi   /* store 1st argument into esi */
    int $TASK_VANISH_INT /* call trap */
    pop %esi            /* restore context */
    ret
