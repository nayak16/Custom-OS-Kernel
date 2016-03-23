/**
 * @file ht.c
 *
 * @brief Implementation of a generic hash table with seperate chaining
 *
 */

#include <stdint.h>
#include <ht.h>
#include <malloc.h>

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
int ht_init(ht_t *t, uint32_t max_size,
            int (*hash)(key_t key), int (*compare)(key_t k1, key_t k2)) {
    if (t == NULL || hash == NULL ||
        max_size == 0 || compare == NULL) return -1;

    /* Create a new array to hold buckets */
    t->arr = calloc(max_size, sizeof(ht_entry_t *));
    if (t->arr == NULL) return -2;

    /* Init all fields */
    t->size = 0;
    t->max_size = max_size;
    t->hash = hash;
    t->compare = compare;

    return 0;
}

int ht_get(ht_t *t, key_t key, void **valp) {
    if (t == NULL || valp == NULL) return -1;

    /* Index into correct bucket */
    int idx = t->hash(key) % t->max_size;
    ht_entry_t *e = t->arr[idx];

    /* Search through chain to find matching key */
    while(e != NULL) {
        if (t->compare(e->key, key) == 0) {
            /* Found */
            *valp = e->val;
            return 0;
        }
        e = e->next;
    }
    /* Not found */
    return -2;
}

int ht_remove(ht_t *t, key_t key, void **valp) {
    if(t == NULL) return -1;

    /* Index into correct bucket */
    int idx = t->hash(key) % t->max_size;
    ht_entry_t *e = t->arr[idx];

    /* Search through chain to find matching key */
    while(e != NULL) {
        if (t->compare(e->key, key) == 0) {
            /* Found */
            /* If needed, save value */
            if (valp != NULL) *valp = e->val;
            /* Remove entry from chain */
            e->prev->next = e->next;
            e->next->prev = e->prev;
            /* Free entry */
            free(e);
            t->size--;
            return 0;
        }
        e = e->next;
    }
    /* Not found */
    return -2;
}

int ht_put(ht_t *t, key_t key, void *val) {
    if (t == NULL) return -1;

    /* Index into correct bucket */
    int idx = t->hash(key) % t->max_size;
    ht_entry_t *e = t->arr[idx];

    /* Loop until end of chain */
    while(e != NULL || e->next != NULL) {
        e = e->next;
    }
    /* Init a new entry */
    ht_entry_t *new_e = malloc(sizeof(ht_entry_t));
    if (new_e == NULL) return -2;
    new_e->val = val;
    new_e->key = key;
    new_e->next = NULL;

    /* Check if first in bucket */
    if (e == NULL) {
        t->arr[idx] = new_e;
        new_e->prev = NULL;
    }/* Otherwise, put at end of chain */
    else {
        e->next = new_e;
        new_e->prev = e;
    }
    t->size++;
    return 0;
}

void ht_destroy(ht_t *t) {
    if (t == NULL) return;

    int i;
    /* Loop through all buckets */
    for (i = 0; i < t->max_size; i++) {
        ht_entry_t *e = t->arr[i];
        ht_entry_t *next;
        /* Loop through all entries in chain */
        while(e != NULL) {
            next = e->next;
            free(e);
            e = next;
        }
    }
    /* Free table */
    free(t->arr);

}



