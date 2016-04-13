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

/**@brief an arbitrary max len */
#define MAX_SYSCALL_PRINT_LEN 512

/** @brief Implements the readline syscall
 *  @param len The maximum characters to read in
 *  @param buf The buffer to read into
 *  @return 0 on success, negative integer code on failure
 */
int syscall_readline_c_handler(int len, char *buf){
    uint32_t max_len;
    keyboard_buffer_size(&keyboard, &max_len);
    if (buf == NULL || len < 0 || (uint32_t)len > max_len) return -1;
    pcb_t *pcb;
    if (scheduler_get_current_pcb(&sched, &pcb) < 0 ) return -2;
    uint32_t priv, access;
    /* Ensure the buffer we are writing to is user rw */
    if (pd_get_permissions(&pcb->pd, (uint32_t)buf, &priv, &access) < 0) return -3;
    if (priv != PRIV_USER && access != ACC_RW) return -4;
    /* attempt to lock the keyboard. only continues when there is an entire
     * line to be read from the keyboard */
    return keyboard_read(&keyboard, len, buf);
}

/** @brief Implements the print system call
 *
 *  Requires the requested length is non-negative and less than our maximum
 *  print length.
 *
 *  @param len The length of the buffer
 *  @param buf The buffer to print
 *  @return 0 on success, -1 on failure
 */
int syscall_print_c_handler(int len, char *buf){
    if (buf == NULL) return -1;
    if (len >= MAX_SYSCALL_PRINT_LEN || len < 0) return -2;
    mutex_lock(&console_lock);
    putbytes(buf, len);
    mutex_unlock(&console_lock);
    return 0;
}


/** @brief Implements the set_term_color system call
 *  @param color The color to set the terminal color to
 *  @return 0 on success, negative integer code on failure
 */
int syscall_set_term_color_c_handler(int color){
    return set_term_color(color);
}

/** @brief Implements the set_cursor_pos system call
 *  @param row The row to set it to
 *  @param col The column to set it to
 *  @return 0 on success, negative integer code on failure
 */
int syscall_set_cursor_pos_c_handler(int row, int col){
    return set_cursor(row, col);
}

/** @brief Implements the get_cursor_pos system call
 *  @param row The row pointer to store to
 *  @param col The column pointer to store to
 *  @return 0 on success, negative integer code on failure
 */
int syscall_get_cursor_pos_c_handler(int *row, int *col){
    return get_cursor(row, col);
}
