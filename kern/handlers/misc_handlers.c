/** @file misc_handlers.c
 *  @brief implements miscellaneous syscall handlers
 *
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)
 *  @bug No known bugs.
 */
#include <string.h>
#include <stdlib.h>
#include <loader.h>

#include <simics.h>

void syscall_halt_c_handler(){
    /* ends simics simulation */
    sim_halt();
}

int syscall_readfile_c_handler(char* filename, char *buf, int count, int offset) {

    /* Check if file name is NULL */
    if (filename == NULL) return -1;

    /* Check if count and offset are positive */
    if (count < 0 || offset < 0) return -2;

    /* Get data from appropriate files */
    return getbytes(filename, offset, count, buf);
}

void syscall_misbehave_c_handler(int mode){
    return;
}
