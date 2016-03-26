/**
 * @file ht.c
 *
 * @brief Implementation of a generic hash table with seperate chaining
 *
 */

#include <stdint.h>
#include <ht.h>
#include <ll.h>
#include <malloc.h>

#include <simics.h>

/**
 * @brief Initializes a hash table with provided parameters
 *
 * @param t ht to init
 * @param max_size max size to make the table
 * @param hash provided hash function
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int ht_init(ht_t *t, uint32_t max_size, int (*hash)(key_t key)) {
    if (t == NULL || hash == NULL || max_size == 0) return -1;

    /* Create a new array to hold buckets */
    t->arr = malloc(max_size * sizeof(ll_t));
    if (t->arr == NULL) return -2;
    int i;

    /* Init all linked list buckets */
    for (i = 0 ; i < max_size ; i++) {
        if (ll_init(&(t->arr[i])) < 0) return -2;
    }

    /* Init all fields */
    t->size = 0;
    t->max_size = max_size;
    t->hash = hash;

    return 0;
}

void *extract_key(void *entry) {

    ht_entry_t *e = (ht_entry_t *) entry;
    return (void*)(e->key);
}

/**
 * @brief Gets the value with the key provided
 *
 * @param t Hashtable to access
 * @param key Key used to find the data desired
 * @param valp Address to store value
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int ht_get(ht_t *t, key_t key, void **valp) {
    if (t == NULL || valp == NULL) return -1;

    /* Index into correct bucket */
    int idx = t->hash(key) % t->max_size;

    ht_entry_t *e;
    /* Find element in bucket */
    if (ll_find(&(t->arr[idx]), extract_key, (void*) key, (void*) &e) < 0) {
        /* Not found */
        return -2;
    }
    *valp = e->val;
    return 0;
}

/**
 * @brief Removes and reports the value with the key provided
 *
 * @param t Hashtable to access
 * @param key Key used to find the data desired
 * @param valp Address to store value
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int ht_remove(ht_t *t, key_t key, void **valp) {
    if(t == NULL) return -1;

    /* Index into correct bucket */
    int idx = t->hash(key) % t->max_size;

    ht_entry_t *e;
    /* Remove from bucket */
    if (ll_remove(&(t->arr[idx]), extract_key, (void*) key, (void**) &e) < 0) {
        /* Not found */
        return -2;
    }
    /* Store value */
    if (valp != NULL) *valp = e->val;

    /* Free entry */
    free(e);
    t->size--;
    return 0;

}

/**
 * @brief Inserts the key and value pair in the specified hash table
 *
 * Uses provided hash function to calculate a bucket index
 *
 * @param t Hash table to insert pair into
 * @param key key in key-value pair
 * @param val value in key-value pair
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int ht_put(ht_t *t, key_t key, void *val) {
    if (t == NULL) return -1;

    void *dummy;
    /* Check if specified key value pair is already in the ht */
    if (ht_get(t, key, &dummy) != -2) return -4;

    /* Index into correct bucket */
    int idx = t->hash(key) % t->max_size;

    /* Init a new entry */
    ht_entry_t *new_e = malloc(sizeof(ht_entry_t));
    if (new_e == NULL) return -2;
    new_e->val = val;
    new_e->key = key;

    if(ll_add_last(&(t->arr[idx]), (void*) new_e) < 0) return -3;
    t->size++;
    return 0;
}

/**
 * @brief Destroys the specified hash table and frees it's data structures
 *
 * @param t Hash table to destroy
 *
 * @return 0 on success, negative error code otherwise
 *
 */
void ht_destroy(ht_t *t) {
    if (t == NULL) return;

    int i;
    /* Free each bucket chain */
    for (i = 0; i < t->max_size; i++) {
        ll_destroy(&(t->arr[i]));
    }
    /* Free table */
    free(t->arr);
}



