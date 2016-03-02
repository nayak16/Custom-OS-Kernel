/** @file syscall_readline.c
 *
 *  @brief
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug
 */

#include <syscall_int.h>

.globl readline


readline:
    push %esi      /* save esi */
    mov %esp, %esi  /* copy stack pointer into esi */
    add $8, %esi     /* increment esi by 8 so it points at 1st argument */
    int $READLINE_INT  /* call trap */
    pop %esi       /* restore esi */
    ret
