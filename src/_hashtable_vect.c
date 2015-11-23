#include "hashtable_vect.h"
#include "logger.h"
#include "hash_funcs.h"
#include "ilog2.h"

static inline void *hashtable_vect_begin(struct hashtable_vect *ht) { return vmbuf_data(&ht->vect); }
static inline void *hashtable_vect_end(struct hashtable_vect *ht) { return vmbuf_wloc(&ht->vect); }
static inline uint32_t hashtable_vect_size(struct hashtable_vect *ht) { return ht->size; }

static inline struct hashtable_vect_internal_entry *hashtable_vect_bucket(struct hashtable_vect *ht, uint32_t b) {
    return ((struct hashtable_vect_internal_entry *)vmbuf_data(&ht->buckets)) + b;
}

static inline void hashtable_vect_move_buckets_range(struct hashtable_vect *ht, uint32_t new_mask, uint32_t begin, uint32_t end) {
    for (; begin < end; ++begin) {
        struct hashtable_vect_internal_entry *ie = hashtable_vect_bucket(ht, begin);
        struct hashtable_vect_entry *e = (struct hashtable_vect_entry *)vmbuf_data_ofs(&ht->entry_buf, ie->entry_ofs);
        uint32_t new_bkt_idx = hashcode(e->key_data, e->key_len) & new_mask;
        /* if already in the right place skip it */
        if (begin == new_bkt_idx)
            continue;
        for (;;) {
            struct hashtable_vect_internal_entry *new_entry = hashtable_vect_bucket(ht, new_bkt_idx);
            if (0 == new_entry->entry_ofs) {
                *new_entry = *ie;
                /* free the bucket so it can be reused */
                ie->entry_ofs = 0;
                break;
            }
            ++new_bkt_idx;
            if (unlikely(new_bkt_idx > new_mask))
                new_bkt_idx = 0;
        }
    }
}

static inline int hashtable_vect_grow(struct hashtable_vect *ht) {
    uint32_t capacity = ht->mask + 1;
    if ((size_t)-1 == vmbuf_alloczero(&ht->buckets, capacity * sizeof(struct hashtable_vect_internal_entry)))
        return LOGGER_ERROR("vmbuf_alloc"), -1;

    uint32_t new_mask = (capacity << 1) - 1;
    uint32_t b = 0;
    //Handle wraparound case
    for (; 0 != hashtable_vect_bucket(ht, b)->entry_ofs; ++b);
    hashtable_vect_move_buckets_range(ht, new_mask, b, capacity);
    hashtable_vect_move_buckets_range(ht, new_mask, 0, b);
    ht->mask = new_mask;
    return 0;
}

static inline int hashtable_vect_check_resize(struct hashtable_vect *ht) {
    if (unlikely(ht->size > (ht->mask >> 1)) && 0 > hashtable_vect_grow(ht))
        return -1;
    return 0;
}

static inline void *_hashtable_vect_insert(struct hashtable_vect *ht, const void *key, uint32_t key_len, int (*alloc_func)()) {
    if (0 > hashtable_vect_check_resize(ht))
        return 0;
    size_t entry_ofs = vmbuf_alloc(&ht->entry_buf, sizeof(struct hashtable_vect_entry) + key_len);

    struct hashtable_vect_entry *e = (struct hashtable_vect_entry *)vmbuf_data_ofs(&ht->entry_buf, entry_ofs);
    e->key_len = key_len;
    e->ofs_val = vmbuf_wlocpos(&ht->vect);
    memcpy(e->key_data, key, key_len);

    uint32_t hc = hashcode(key, key_len);
    uint32_t b = hc & ht->mask;
    for(;;) {
        struct hashtable_vect_internal_entry *ie = hashtable_vect_bucket(ht, b);
        if (0 == ie->entry_ofs) {
            ie->hashcode = hc;
            ie->entry_ofs = entry_ofs;
            ht->size++;
            if (0 > alloc_func())
                return NULL;
            return vmbuf_data_ofs(&ht->vect, e->ofs_val);
        }
        b++;
        if (unlikely(b > ht->mask))
            b = 0;
    }
}

static inline void *hashtable_vect_insert(struct hashtable_vect *ht, const void *key, uint32_t key_len, const void *val) {
    inline int alloc_func() {
        return vmbuf_memcpy(&ht->vect, val, ht->val_size);
    }
    return _hashtable_vect_insert(ht, key, key_len, alloc_func);
}

static inline void *hashtable_vect_insert_alloc(struct hashtable_vect *ht, const void *key, uint32_t key_len) {
    inline int alloc_func() {
        return vmbuf_alloc(&ht->vect, ht->val_size);
    }
    return _hashtable_vect_insert(ht, key, key_len, alloc_func);
}

static inline void *hashtable_vect_lookup(struct hashtable_vect *ht, const void *key, uint32_t key_len) {
    uint32_t hc = hashcode(key, key_len);
    uint32_t b = hc & ht->mask;
    for (;;) {
        struct hashtable_vect_internal_entry *ie = hashtable_vect_bucket(ht, b);
        if (0 == ie->entry_ofs)
            return NULL;
        if (ie->hashcode == hc) {
            struct hashtable_vect_entry *e = (struct hashtable_vect_entry *)vmbuf_data_ofs(&ht->entry_buf, ie->entry_ofs);
            if (e->key_len == key_len && 0 == memcmp(e->key_data, key, key_len))
                return vmbuf_data_ofs(&ht->vect, e->ofs_val);
        }
        b++;
        if (unlikely(b > ht->mask))
            b = 0;
    }
    return NULL;
}
