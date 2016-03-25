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

/* access to console mutex and keyboard buffer */
#include <kern_internals.h>

/* access to mutex */
#include <mutex.h>

/* access to KH functions */
#include <x86/keyhelp.h>

/* an arbitrary max len */
//TODO: figure out these lengths
#define MAX_SYSCALL_PRINT_LEN 512


int syscall_readline_c_handler(int len, char *buf){
    uint32_t max_len;
    keyboard_buffer_size(&keyboard, &max_len);
    //TODO: check that buf is not in read only section
    if (buf == NULL || len > max_len);
        return -1;
    /* attempt to lock the keyboard. only continues when there is an entire
     * line to be read from the keyboard */
    return keyboard_read(&keyboard, len, buf);
}

int syscall_print_c_handler(int len, char *buf){
    if (buf == NULL) return -1;
    if (len >= MAX_SYSCALL_PRINT_LEN || len < 0) return -2;
    mutex_lock(&console_lock);
    putbytes(buf, len);
    mutex_unlock(&console_lock);
    return 0;
}

int syscall_set_term_color_c_handler(int color){
    return set_term_color(color);
}

int syscall_set_cursor_pos_c_handler(int row, int col){
    return set_cursor(row, col);
}

int syscall_get_cursor_pos_c_handler(int *row, int *col){
    return get_cursor(row, col);
}
