/** @file syscall_new_pages.c
 *
 *  @brief
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug
 */

#include <syscall_int.h>

.globl new_pages


new_pages:
    push %esi      /* save esi */
    mov %esp, %esi  /* copy stack pointer into esi */
    add $8, %esi     /* increment esi by 8 so it points at 1st argument */
    int $NEW_PAGES_INT  /* call trap */
    pop %esi       /* restore esi */
    ret
