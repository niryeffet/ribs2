struct tht_entry {
    uint32_t hashcode;
    void *rec;
};

struct thashtable {
    uint32_t mask;
    uint32_t size;
    struct tht_entry *buckets[32];
};

struct thashtable_internal_rec {
    uint32_t key_size;
    uint32_t val_size;
    char data[];
};

static inline void *thashtable_get_key(thashtable_rec_t *rec) {
    return ((struct thashtable_internal_rec *)rec)->data;
}

static inline uint32_t thashtable_get_key_size(thashtable_rec_t *rec) {
    return ((struct thashtable_internal_rec *)rec)->key_size;
}

static inline void *thashtable_get_val(thashtable_rec_t *rec) {
    return ((struct thashtable_internal_rec *)rec)->data + ((struct thashtable_internal_rec *)rec)->key_size;
}

static inline uint32_t thashtable_get_val_size(thashtable_rec_t *rec) {
    return ((struct thashtable_internal_rec *)rec)->val_size;
}

static inline uint32_t thashtable_get_size(struct thashtable *tht) {
    return tht->size;
}

static inline const char *thashtable_lookup_str(struct thashtable *tht, const char *key, const char *default_val) {
    thashtable_rec_t *rec = thashtable_lookup(tht, key, strlen(key));
    return (rec ? thashtable_get_val(rec) : default_val);
}
