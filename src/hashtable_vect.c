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
#include "file_utils.h"
#include <limits.h>
#include <stdio.h>

int _hashtable_vect_init(struct hashtable_vect *ht, uint32_t initial_size, size_t val_size) {
    if (0 == initial_size)
        initial_size = 64;
    initial_size = next_p2(initial_size) << 1;
    vmallocator_alloczero(&ht->buckets, sizeof(struct hashtable_vect_header) + initial_size * sizeof(struct hashtable_vect_internal_entry));
    vmallocator_check_resize(&ht->entry_buf, initial_size * sizeof(struct hashtable_vect_entry));
    vmallocator_alloc(&ht->entry_buf, sizeof(struct hashtable_vect_entry)); //First offset is reserved to indicate unused entry
    vmallocator_check_resize(&ht->vect, val_size * initial_size);

    _HASHTABLE_VECT_HEADER()->val_size = val_size;
    _HASHTABLE_VECT_HEADER()->mask = initial_size - 1;
    _HASHTABLE_VECT_HEADER()->size = 0;
    return 0;
}

int hashtable_vect_init(struct hashtable_vect *ht, uint32_t initial_size, size_t val_size) {
    if (0 > vmallocator_init(&ht->entry_buf)
            || 0 > vmallocator_init(&ht->buckets)
            || 0 > vmallocator_init(&ht->vect))
        return LOGGER_ERROR("vmallocator_init"), -1;
    return _hashtable_vect_init(ht, initial_size, val_size);
}

#define HASHTABLE_VECT_OPEN_VMALLOC(VMALLOC, FILENAME) \
    if (PATH_MAX <= snprintf(path, PATH_MAX, "%s/"FILENAME, dirname)) \
        return LOGGER_ERROR_FUNC("filename too long"), -1; \
    if (0 > vmallocator_open(&VMALLOC, path, flags)) \
        return -1

int hashtable_vect_open(struct hashtable_vect *ht, uint32_t initial_size, size_t val_size, const char *dirname, int flags) {
    if (0 > mkdir_recursive(dirname))
        return LOGGER_ERROR_FUNC("mkdir_recursive"), -1;

    char path[PATH_MAX];
    HASHTABLE_VECT_OPEN_VMALLOC(ht->buckets, "buckets");
    HASHTABLE_VECT_OPEN_VMALLOC(ht->entry_buf, "entries");
    HASHTABLE_VECT_OPEN_VMALLOC(ht->vect, "vect");

    // Recover in case of crash - vmallocator_close wasn't called so the files weren't truncated
    // hashtable_vect_end will return the wrong position
    vmallocator_wlocset(&ht->entry_buf, _HASHTABLE_VECT_HEADER()->size * sizeof(struct hashtable_vect_entry));
    vmallocator_wlocset(&ht->vect, _HASHTABLE_VECT_HEADER()->size * _HASHTABLE_VECT_HEADER()->val_size);

    if (0 == _HASHTABLE_VECT_HEADER()->mask)
        return _hashtable_vect_init(ht, initial_size, val_size);
    return 0;
}

int hashtable_vect_free(struct hashtable_vect *ht) {
    vmallocator_free(&ht->entry_buf);
    vmallocator_free(&ht->buckets);
    vmallocator_free(&ht->vect);
    return 0;
}

int hashtable_vect_close(struct hashtable_vect *ht) {
    return vmallocator_close(&ht->entry_buf)
            | vmallocator_close(&ht->buckets)
            | vmallocator_close(&ht->vect);
}
