/** @file syscall_get_ticks.S
 *
 *  @brief
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug
 */

#include <syscall_int.h>

.globl get_ticks

get_ticks:
    int $GET_TICKS_INT
    ret
