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

#include "hashtable_vect.h"
#include "logger.h"

int hashtable_vect_init(struct hashtable_vect *ht, uint32_t initial_size, size_t val_size) {
    if (0 == initial_size)
        initial_size = 64;
    initial_size = next_p2(initial_size) << 1;
    if (0 > vmbuf_init(&ht->entry_buf, initial_size * sizeof(struct hashtable_vect_entry))
            || 0 > vmbuf_init(&ht->buckets, initial_size * sizeof(struct hashtable_vect_internal_entry))
            || 0 > vmbuf_init(&ht->vect, val_size * initial_size))
        return LOGGER_ERROR("vmbuf_init"), -1;
    vmbuf_alloczero(&ht->buckets, initial_size * sizeof(struct hashtable_vect_internal_entry));
    vmbuf_alloc(&ht->entry_buf, sizeof(struct hashtable_vect_entry)); //First offset is reserved to indicate unused entry
    ht->val_size = val_size;
    ht->mask = initial_size - 1;
    ht->size = 0;
    return 0;
}

int hashtable_vect_free(struct hashtable_vect *ht) {
    return vmbuf_free(&ht->entry_buf)
            | vmbuf_free(&ht->buckets)
            | vmbuf_free(&ht->vect);
}
