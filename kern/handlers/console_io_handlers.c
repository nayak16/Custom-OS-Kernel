/** @file console_io_handlers.c
 *  @brief implements thread management syscall handlers
 *
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)
 *  @bug No known bugs.
 */


#include <simics.h>
#include <stdlib.h>
#include <console.h>

/* access to console mutex */
#include <kern_internals.h>

/* access to mutex */
#include <mutex.h>

/* an arbitrary max len */
#define MAX_SYSCALL_PRINT_LEN 512

int syscall_print_c_handler(int len, char *buf){
    if (buf == NULL) return -1;
    if (len >= MAX_SYSCALL_PRINT_LEN || len < 0) return -2;
    //mutex_lock(&console_lock);
    putbytes(buf, len);
    //mutex_unlock(&console_lock);
    return 0;
}

