/** @file syscall_yield.S
 *
 *  @brief implements yield stub
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug No known bugs
 */

#include <syscall_int.h>

.globl yield

yield:
    push %esi           /* save context */
    mov 8(%esp), %esi   /* store 1st argument into esi */
    int $YIELD_INT /* call trap */
    pop %esi            /* restore context */
    ret
