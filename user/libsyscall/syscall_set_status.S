/** @file syscall_set_status.S
 *
 *  @brief implements set_status stub
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug No known bugs
 */

#include <syscall_int.h>

.globl set_status

set_status:
    push %esi           /* save context */
    mov 8(%esp), %esi   /* store 1st argument into esi */
    int $SET_STATUS_INT /* call trap */
    pop %esi            /* restore context */
    ret
