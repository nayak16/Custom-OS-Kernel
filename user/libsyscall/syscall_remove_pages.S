/** @file syscall_remove_pages.c
 *
 *  @brief
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug
 */

#include <syscall_int.h>

.globl remove_pages

remove_pages:
    push %esi           /* save context */
    mov 8(%esp), %esi   /* store 1st argument into esi */
    int $REMOVE_PAGES_INT /* call trap */
    pop %esi            /* restore context */
    ret
