/** @file syscall_fork.S
 *
 *  @brief implements fork stub
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug No known bugs
 */

#include <syscall_int.h>

.globl fork

fork:
    int $FORK_INT
    ret
