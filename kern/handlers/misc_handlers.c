/** @file misc_handlers.c
 *  @brief implements miscellaneous syscall handlers
 *
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)
 *  @bug No known bugs.
 */

#include <simics.h>

void syscall_halt_c_handler(){
    /* ends simics simulation */
    sim_halt();
}
