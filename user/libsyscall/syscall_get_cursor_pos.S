/** @file syscall_get_cursor_pos.S
 *
 *  @brief
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug
 */

#include <syscall_int.h>

.globl get_cursor_pos


get_cursor_pos:
    push %esi      /* save esi */
    mov %esp, %esi  /* copy stack pointer into esi */
    add $8, %esi     /* increment esi by 8 so it points at 1st argument */
    int $GET_CURSOR_POS_INT  /* call trap */
    pop %esi       /* restore esi */
    ret
