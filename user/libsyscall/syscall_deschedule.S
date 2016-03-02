/** @file syscall_deschedule.S
 *
 *  @brief implements deschedule stub
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug No known bugs
 */

#include <syscall_int.h>

.globl deschedule

deschedule:
    push %esi           /* save context */
    mov 8(%esp), %esi   /* store 1st argument into esi */
    int $DESCHEDULE_INT /* call trap */
    pop %esi            /* restore context */
    ret
