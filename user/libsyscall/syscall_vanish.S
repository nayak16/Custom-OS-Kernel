/** @file syscall_vanish.S
 *
 *  @brief implements vanish stub
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug No known bugs
 */

#include <syscall_int.h>

.globl vanish

vanish:
    int $VANISH_INT
    ret
