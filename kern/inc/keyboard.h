/** @file keyboard.h
 *  @brief Defines interface for a keyboard manager
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs
 */
#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include <stdlib.h>
#include <circ_buffer.h>
#include <mutex.h>

#define KEYBOARD_OPEN 1
#define KEYBOARD_BLOCKED 0

typedef struct keyboard {
    /** @brief the character buffer to be stored into */
    circ_buf_t *buf;
    /** @brief the number of threads waiting on the keyboard */
    int num_waiting;
    /** @brief mutex to protect buf and num_waiting */
    mutex_t m;
} keyboard_t;

int keyboard_init(keyboard_t *k, uint32_t len);
void keyboard_destroy(keyboard_t *k);
int keyboard_write(keyboard_t *k, uint32_t aug_char);
int keyboard_read(keyboard_t *k, uint32_t *aug_char);
int keyboard_open(keyboard_t *k);
int keyboard_close(keyboard_t *k);
#endif /* _KEYBOARD_H_ */
