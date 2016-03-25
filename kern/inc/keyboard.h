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
#include <sem.h>

typedef struct keyboard {
    /** @brief the character buffer to be stored into */
    circ_buf_t *buf;
    /** @brief mutex to protect buf */
    mutex_t m;
    /** @brief semaphore to keep track of avaliable resources
     *  each resource being 1 full line ended by newline */
    sem_t sem;
} keyboard_t;

int keyboard_init(keyboard_t *k, uint32_t len);
void keyboard_destroy(keyboard_t *k);
int keyboard_write(keyboard_t *k, uint32_t val);
int keyboard_read(keyboard_t *k, int len, char *buf);
int keyboard_buffer_size(keyboard_t *k, uint32_t *len);
#endif /* _KEYBOARD_H_ */
