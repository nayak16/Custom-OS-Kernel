/** @file syscall_wait.S
 *
 *  @brief implements wait stub
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug No known bugs
 */

#include <syscall_int.h>

.globl wait

wait:
    push %esi           /* save context */
    mov 8(%esp), %esi   /* store 1st argument into esi */
    int $WAIT_INT /* call trap */
    pop %esi            /* restore context */
    ret
