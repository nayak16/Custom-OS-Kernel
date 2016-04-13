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
/** @brief Implements the halt system call
 *
 *  For non simics this does nothing, the halt comes after in the assembly
 *  wrapper
 *
 *  @return Does not return
 */
void syscall_halt_c_handler(){
    /* ends simics simulation */
    sim_halt();
}

/** @brief Implements the readfile system call
 *  @param filename The filename to read
 *  @param buf The buffer to read into
 *  @param count The number of bytes to read
 *  @param offset The offset into the file
 *  @return 0 on success, negative integer code on failure
 */
int syscall_readfile_c_handler(char* filename, char *buf, int count, int offset) {

    /* Check if file name is NULL */
    if (filename == NULL) return -1;

    /* Check if count and offset are positive */
    if (count < 0 || offset < 0) return -2;

    /* Get data from appropriate files */
    return getbytes(filename, offset, count, buf);
}

/** @brief Implements the misbehave system call
 *
 *  Does nothing lol.
 *
 *  @param mode The mode to switch to
 *  @return Void
 */
void syscall_misbehave_c_handler(int mode){
    return;
}
