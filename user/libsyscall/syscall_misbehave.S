/** @file syscall_misbehave.S
 *
 *  @brief implements misbehave stub
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug No known bugs
 */

#include <syscall_int.h>

.globl misbehave

misbehave:
    push %esi           /* save context */
    mov 8(%esp), %esi   /* store 1st argument into esi */
    int $MISBEHAVE_INT /* call trap */
    pop %esi            /* restore context */
    ret
