/**
 * @file ht.h
 *
 * @brief Interface to a generic hash table with seperate chaining
 *
 */

#ifndef _HT_H_
#define _HT_H_

#include <stdint.h>
#include <stdlib.h>
#include <ll.h>
#include <circ_buffer.h>

/**
 * @brief typedef of a key
 */
typedef int key_t;

/**
 * @brief
 */
typedef struct ht_entry {
    key_t key;
    void *val;
} ht_entry_t;

typedef struct ht {
    uint32_t size;
    uint32_t max_size;
    int (*hash)(key_t key);
    ll_t *arr;
} ht_t;

int ht_init(ht_t *t, uint32_t max_size, int (*hash)(key_t key));
int ht_get(ht_t *t, key_t key, void **valp);
int ht_remove(ht_t *t, key_t key, void **valp, circ_buf_t *addrs_to_free);
int ht_put(ht_t *t, key_t key, void *val);
void ht_destroy(ht_t *t);

#endif /* _HT_H_ */

