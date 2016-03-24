/** @file keyboard.h
 *  @brief Defines interface for a keyboard manager
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs
 */

#include <keyboard.h>
#include <circ_buffer.h>
#include <stdlib.h>

#include <simics.h>

int keyboard_init(keyboard_t *k, uint32_t len){
    if (k == NULL || len == 0) return -1;
    circ_buf_t *cbuf = malloc(sizeof(circ_buf_t));
    if (circ_buf_init(cbuf, len) < 0) return -2;
    k->buf = cbuf;
    k->num_waiting = 0;
    return 0;
}

void keyboard_destroy(keyboard_t *k){
    circ_buf_destroy(k->buf);
    free(k->buf);
}

int keyboard_write(keyboard_t *k, uint32_t aug_char){
    if (k == NULL) return -1;
    if (k->num_waiting > 0) return -1;
    return circ_buf_write(k->buf, (void *)aug_char);
}

int keyboard_read(keyboard_t *k, uint32_t *aug_char){
    if (k == NULL || aug_char == NULL) return -1;
    return circ_buf_read(k->buf, (void **)aug_char);
}

int keyboard_open(keyboard_t *k){
    if (k == NULL) return -1;
    k->num_waiting++;
    return 0;
}

int keyboard_close(keyboard_t *k){
    if (k == NULL) return -1;
    k->num_waiting--;
    return 0;
}


