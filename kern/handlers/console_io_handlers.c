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
#define MAX_SYSCALL_READLINE_LEN 512

int readchar(int *result){
    if (result == NULL) return -1;
    uint32_t aug_char;
    kh_type proc_char;
    /* no keys to be read */
    if (keyboard_read(&keyboard, &aug_char) < 0) return -1;
    /* bit extend with 0's before returning */
    proc_char = process_scancode(aug_char);
    if (KH_HASDATA(proc_char) && KH_ISMAKE(proc_char)){
        *result = KH_GETCHAR(proc_char);
        return 0;
    }
    /* key stroke is a non-data or non-keystroke-down key press */
    return -1;
}

int syscall_readline_c_handler(int len, char *buf){
    //TODO: check that buf is not in read only section
    if (buf == NULL || len >= MAX_SYSCALL_READLINE_LEN)
        return -1;
    keyboard_open(&keyboard);
    //deschedule
    //uint32_t i, aug_char;
    //read in until len or new line
    keyboard_close(&keyboard);
    return 0;
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
