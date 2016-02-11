/*
    This file is part of RIBS2.0 (Robust Infrastructure for Backend Systems).
    RIBS is an infrastructure for building great SaaS applications (but not
    limited to).

    Copyright (C) 2015 TrueSkills, Inc.

    RIBS is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, version 2.1 of the License.

    RIBS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with RIBS.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _HASHTABLE_VECT__H_
#define _HASHTABLE_VECT__H_

#include <stdint.h>
#include "vmbuf.h"

/*
 * hashtable_vect is useful for applications that, after inserting values into a hashtable,
 * then want to loop over the inserted values. It stores all values sequentially in a secondary
 * buffer with no holes. The main hashtable buffer stores only offsets into this value buffer, not
 * the values themselves. This main buffer is where the holes are. A side effect of this is that
 * deleting from a hashtable_vect is very expensive. Hence it is unimplemented.
 */
struct hashtable_vect {
    struct vmbuf entry_buf;
    struct vmbuf buckets;
    struct vmbuf vect;
    uint32_t mask;
    uint32_t size;
    size_t val_size;
};

struct hashtable_vect_internal_entry {
    uint32_t hashcode;
    uint32_t entry_ofs;
};

struct hashtable_vect_entry {
    uint32_t ofs_val;
    uint32_t key_len;
    char key_data[];
};


#define HASHTABLE_VECT_INITIALIZER {VMBUF_INITIALIZER, VMBUF_INITIALIZER, VMBUF_INITIALIZER, 0, 0, 0}
#define HASHTABLE_VECT_MAKE(x) (x) = (struct hashtable_vect)HASHTABLE_VECT_INITIALIZER

int hashtable_vect_init(struct hashtable_vect *ht, uint32_t n, size_t val_size);
int hashtable_vect_free(struct hashtable_vect *ht);
static inline void *hashtable_vect_begin(struct hashtable_vect *ht);
static inline void *hashtable_vect_end(struct hashtable_vect *ht);
static inline uint32_t hashtable_vect_size(struct hashtable_vect *ht);
static inline void *hashtable_vect_insert(struct hashtable_vect *ht, const void *key, uint32_t key_len, const void *val);
static inline void *hashtable_vect_insert_alloc(struct hashtable_vect *ht, const void *key, uint32_t key_len);
static inline void *hashtable_vect_lookup(struct hashtable_vect *ht, const void *key, uint32_t key_len);

#include "../src/_hashtable_vect.c"

#endif // _HASHTABLE_VECT__H_
