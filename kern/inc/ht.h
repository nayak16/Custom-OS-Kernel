/**
 * @file ht.h
 *
 * @brief Interface to a generic hash table with seperate chaining
 *
 * @author Aatish Nayak (aatishn)
 * @bug No known bugs
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
 * @brief defines an entry into a hash table
 */
typedef struct ht_entry {
    /** @brief key to the entry */
    key_t key;
    /** @brief value stored */
    void *val;
} ht_entry_t;

typedef struct ht {
    /** @brief number of entries in hash table */
    uint32_t size;
    /** @brief number of buckets in hash table */
    uint32_t max_size;
    /** @brief hashing function used */
    int (*hash)(key_t key);
    /** @brief array of buckets */
    ll_t *arr;
} ht_t;

int ht_init(ht_t *t, uint32_t max_size, int (*hash)(key_t key));
int ht_get(ht_t *t, key_t key, void **valp);
int ht_remove(ht_t *t, key_t key, void **valp, circ_buf_t *addrs_to_free);
int ht_put(ht_t *t, key_t key, void *val);
int ht_put_entry(ht_t *t, ht_entry_t *entry, ll_node_t *entry_node);
void ht_destroy(ht_t *t);

#endif /* _HT_H_ */

