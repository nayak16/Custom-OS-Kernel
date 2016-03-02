/** @file syscall_set_term_color.c
 *
 *  @brief
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug
 */

#include <syscall_int.h>

.globl set_term_color

set_term_color:
    push %esi               /* save context */
    mov 8(%esp), %esi       /* store 1st argument into esi */
    int $SET_TERM_COLOR_INT /* call trap */
    pop %esi                /* restore context */
    ret
