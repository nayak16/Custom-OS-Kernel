/** @file circ_buffer.h
 *  @brief Defines interface for a circular buffer
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)

 *  @bug No known bugs
 */


#ifndef _CIRC_BUFFER_H_
#define _CIRC_BUFFER_H_

#include <stdint.h>

typedef struct circular_buffer{
  /** @brief the read index of the buffer */
  uint32_t i_r;
  /** @brief the write index of the buffer */
  uint32_t i_w;
  /** @brief the internal buffer */
  void **buf;
  /** @brief the length of the buffer */
  uint32_t len;
  /** @brief the number of elements in the buffer */
  uint32_t n;
} circ_buf_t;

int circ_buf_init(circ_buf_t *circ_buf, uint32_t len);
void circ_buf_destroy(circ_buf_t *circ_buf);
int circ_buf_write(circ_buf_t *circ_buf, void *v);
int circ_buf_read(circ_buf_t *circ_buf, void **v);
int circ_buf_size(circ_buf_t *circ_buf, uint32_t *len);
int circ_buf_count(circ_buf_t *circ_buf, uint32_t *n);
int circ_buf_delete_front(circ_buf_t *circ_buf);
#endif /* _CIRC_BUFFER_H_ */
