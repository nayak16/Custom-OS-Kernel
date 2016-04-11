/** @file circ_buffer.c
 *  @brief implements a circular buffer
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs
 */

#include <circ_buffer.h>
#include <stdlib.h>

/* Implementation */

/** @brief Initializes a circular buffer of size len
 *  @param circ_buf The circular buffer
 *  @param len The maximum length to cap it at
 *  @return 0 on success, negative integer code on failure
 */
int circ_buf_init(circ_buf_t *circ_buf, uint32_t len){
    if (circ_buf == NULL || len == 0) return -1;
    circ_buf->i_r = 0;
    circ_buf->i_w = 0;
    circ_buf->n = 0;
    circ_buf->buf = (void **)malloc(len * sizeof(void *));
    if (circ_buf->buf == NULL) return -2;
    circ_buf->len = len;
    return 0;
}

/** @brief Destroys a circular buffer
 *  @param circ_buf The circular buffer
 *  @return Void
 */
void circ_buf_destroy(circ_buf_t *circ_buf){
    if (circ_buf == NULL) return;
    free((void *)circ_buf->buf);
    return;
}

/** @brief Writes a value to the circular buffer
 *  @param circ_buf The circular buffer
 *  @param v The value to write
 *  @return 0 on success, negative integer code on failure
 */
int circ_buf_write(circ_buf_t *circ_buf, void *v){
    if (circ_buf == NULL) return -1;
    /* move i_r to next spot*/
    int next_i_w;
    next_i_w = (circ_buf->i_w + 1) % circ_buf->len;
    if (circ_buf->n+1 == circ_buf->len){
        /* no space left, ignore character */
        return -1;
    } else {
        /* save character */
        circ_buf->buf[circ_buf->i_w] = v;
        /* advance i_w */
        circ_buf->i_w = next_i_w;
        /* increase the size */
        circ_buf->n++;
        return 0;
    }
}

/** @brief Reads a value out of the circular buffer
 *  @param circ_buf The circular buffer
 *  @param v The value to read into
 *  @return 0 on success, negative integer code on failure
 */
int circ_buf_read(circ_buf_t *circ_buf, void **v){
    if (circ_buf == NULL) return -1;
    if (circ_buf->n == 0) return -2;
    /* optionally save character into v */
    if (v != NULL) *v = circ_buf->buf[circ_buf->i_r];
    /* update read position */
    circ_buf->i_r = (circ_buf->i_r + 1) % circ_buf->len;
    /* decrease count */
    circ_buf->n--;
    return 0;
}

/** @brief Removes the most recently written value in the circular buffer
 *  @param circ_buf The circular buffer
 *  @return 0 on success, negative integer code on failure
 */
int circ_buf_delete_front(circ_buf_t *circ_buf){
    if (circ_buf == NULL) return -1;
    if (circ_buf->n == 0) return -2;
    circ_buf->i_w = (circ_buf->i_w - 1) % circ_buf->len;
    circ_buf->n--;
    return 0;
}

/** @brief Returns the number of values in the circular buffer
 *  @param circ_buf The circular buffer
 *  @return 0 on success, negative integer code on failure
 */
int circ_buf_count(circ_buf_t *circ_buf, uint32_t *count){
    if (circ_buf == NULL) return -1;
    *count = circ_buf->n;
    return 0;
}

/** @brief Gets the maximum capacity of a circular buffer
 *  @param circ_buf The circular buffer
 *  @param len The address to store the result in
 *  @return 0 on success, negative integer code on failure
 */
int circ_buf_size(circ_buf_t *circ_buf, uint32_t *len){
    if (circ_buf == NULL || len == NULL) return -1;
    *len = circ_buf->len;
    return 0;
}
