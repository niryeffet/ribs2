/*
    This file is part of RIBS2.0 (Robust Infrastructure for Backend Systems).
    RIBS is an infrastructure for building great SaaS applications (but not
    limited to).

    Copyright (C) 2012,2013 Adap.tv, Inc.

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
#ifndef _CONTEXT_STRUCT__H_
#define _CONTEXT_STRUCT__H_

struct ribs_context {
    uint64_t x19, x20; // 0
    uint64_t x21, x22; // 16
    uint64_t x23, x24; // 32
    uint64_t x25, x26; // 48
    uint64_t x27, x28; // 64

    uint64_t x29;       // 80 frame
    uint64_t x30;       // 88 link
    uint64_t sp;        // 96 stack

    struct ribs_context *next_free;
    struct memalloc memalloc;
    uint32_t ribify_memalloc_refcount;
    char reserved[];
};

#endif // _CONTEXT_STRUCT__H_
