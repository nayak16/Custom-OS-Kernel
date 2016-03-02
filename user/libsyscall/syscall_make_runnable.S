/** @file syscall_make_runnable.S
 *
 *  @brief implements make_runnable stub
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug No known bugs
 */

#include <syscall_int.h>

.globl make_runnable

make_runnable:
    push %esi           /* save context */
    mov 8(%esp), %esi   /* store 1st argument into esi */
    int $MAKE_RUNNABLE_INT /* call trap */
    pop %esi            /* restore context */
    ret
