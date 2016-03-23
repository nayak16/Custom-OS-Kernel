/** @file circ_buffer.c
 *  @brief implements a circular buffer
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs
 */

#include <circ_buffer.h>
#include <stdlib.h>

/* Implementation */
int circ_buf_init(circ_buf_t *circ_buf, uint32_t len){
    if (circ_buf == NULL || len == 0) return -1;
    circ_buf->i_r = 0;
    circ_buf->i_w = 0;
    circ_buf->buf = (void **)malloc(len * sizeof(void *));
    if (circ_buf->buf == NULL) return -2;
    circ_buf->len = len;
    return 0;
}

void circ_buf_destroy(circ_buf_t *circ_buf){
    if (circ_buf == NULL) return;
    free((void *)circ_buf->buf);
    return;
}

int circ_buf_write(circ_buf_t *circ_buf, void *v){
  /* move i_r to next spot*/
    int next_i_w;
    next_i_w = (circ_buf->i_w + 1) % circ_buf->len;
    if (next_i_w == circ_buf->i_r){
        /* no space left, ignore character */
        return -1;
    } else {
        /* save character */
        circ_buf->buf[circ_buf->i_w] = v;
        /* advance i_w */
        circ_buf->i_w = next_i_w;
        return 0;
    }
}

int circ_buf_read(circ_buf_t *circ_buf, void **v){
    if (circ_buf->i_r == circ_buf->i_w){
        return -1; // no new data
    }
    *v = circ_buf->buf[circ_buf->i_r];
    /* update read position */
    circ_buf->i_r = (circ_buf->i_r + 1) % circ_buf->len;
    return 0;
}
