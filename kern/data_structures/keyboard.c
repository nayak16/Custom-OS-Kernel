/** @file keyboard.c
 *
 *  We designed the keyboard as a combination of a semaphore and a circular
 *  buffer. First, we collect keyboard strokes from the keyboard handler which
 *  are first processed and then stored into a keyboard buffer (implemented as a
 *  circular buffer). Upon receiving a new line, we can signal to any waiting
 *  threads that have called keyboard_read and wake up the next thread in the
 *  semaphore queue. This allows for lines of characters to be remembered even
 *  if no call to keyboard_read exists yet.
 *
 *  @brief Defines interface for a keyboard manager
 *  @author Christopher Wei (cjwei)
 *  @bug No handling of buffer overflow
 */

#include <keyboard.h>
#include <circ_buffer.h>
#include <stdlib.h>
#include <string.h>
#include <simics.h>

/** @brief Initializes a keyboard
 *  @param k The keyboard
 *  @param len The size of the keyboard buffer
 *  @return 0 on success, negative integer code on failure
 */
int keyboard_init(keyboard_t *k, uint32_t len){
    if (k == NULL || len == 0) return -1;
    circ_buf_t *cbuf = malloc(sizeof(circ_buf_t));
    if (circ_buf_init(cbuf, len) < 0) return -2;
    if (sem_init(&(k->sem), 0) < 0) return -3;
    if (mutex_init(&(k->m)) < 0) return -4;
    k->buf = cbuf;
    return 0;
}

/** @brief Destroys a keyboard
 *  @param k The keyboard
 *  @return Void
 */
void keyboard_destroy(keyboard_t *k){
    circ_buf_destroy(k->buf);
    mutex_destroy(&(k->m));
    sem_destroy(&(k->sem));
    free(k->buf);
}

/** @brief Writes a value to the keyboard
 *  @param k The keyboard
 *  @param len The value
 *  @return 0 on success, negative integer code on failure
 */
int keyboard_write(keyboard_t *k, uint32_t val){
    if (k == NULL) return -1;
    mutex_lock(&(k->m));
    if (circ_buf_write(k->buf, (void *)val) < 0) return -2;
    if ((char)val == '\n') sem_signal(&k->sem);
    mutex_unlock(&(k->m));
    return 0;
}

/** @brief Destructively reads a value from the keyboard
 *  @param k The keyboard
 *  @param len The number of characters to read
 *  @param buf The character buffer to read into
 *  @return 0 on success, negative integer code on failure
 */
int keyboard_read(keyboard_t *k, int len, char *buf){
    if (k == NULL || len <= 0 || buf == NULL) return -1;
    sem_wait(&k->sem);
    mutex_lock(&(k->m));
    /* read the line in from circ buf */
    char temp_buf[len];
    uint32_t i, val;
    for (i = 0; i < len; i++){
        if (circ_buf_read(k->buf, (void **)&val) < 0){
            mutex_unlock(&(k->m));
            return -2;
        }
        if ((char)val == '\n') break;
        temp_buf[i] = (char)val;
    }
    memcpy(buf, temp_buf, i*sizeof(char));
    mutex_unlock(&(k->m));
    return i;
}

/** @brief Gets the size of the keyboard buffer
 *  @param k The keyboard
 *  @param len The pointer to store the size of the keyboard buffer
 *  @return 0 on success, negative integer code on failure
 */
int keyboard_buffer_size(keyboard_t *k, uint32_t *len){
    if (k == NULL || len == NULL) return -1;
    return circ_buf_size(k->buf, len);
}
