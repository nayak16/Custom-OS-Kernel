/** @file syscall_getchar.S
 *
 *  @brief
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug
 */

#include <syscall_int.h>

.globl getchar

getchar:
    int $GETCHAR_INT
    ret
