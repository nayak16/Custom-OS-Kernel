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

    /* Check if buf is large enough to store count bytes */
    if (strlen(buf) != count) return -3;

    /* Check if all files are requested */
    if (strcmp(filename, ".") == 0) {
        return get_all_files(buf, strlen(buf));
    } else {
        MAGIC_BREAK;
        /* Particular file requested */
    }
    return -1;
}
