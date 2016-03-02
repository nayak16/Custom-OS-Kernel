/** @file syscall_exec.S
 *
 *  @brief implements exec stub
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug No known bugs
 */

#include <syscall_int.h>

.globl exec


exec:
    push %esi        /* save esi */
    mov %esp, %esi   /* copy stack pointer into esi */
    add $8, %esi     /* increment esi by 8 so it points at 1st argument */
    int $EXEC_INT  /* call trap */
    pop %esi         /* restore esi */
    ret
