/*
    This file is part of RIBS2.0 (Robust Infrastructure for Backend Systems).
    RIBS is an infrastructure for building great SaaS applications (but not
    limited to).

    Copyright (C) 2013,2014 Adap.tv, Inc.

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
#include "ribs_zlib.h"
#include <zlib.h>
#include <limits.h>

static void *_zalloc(void *_vmb, uint32_t items, uint32_t size) {
    struct vmbuf *vmb = _vmb;
    return vmbuf_allocptr(vmb, items * size);
}

static void _zfree(void *_vmb UNUSED_ARG, void *ptr UNUSED_ARG) {
}

static inline void _init_alloc(z_stream *strm) {
    static struct vmbuf zalloc_buf = VMBUF_INITIALIZER;
    vmbuf_init(&zalloc_buf, 1024*1024*2);
    strm->zalloc = _zalloc;
    strm->zfree = _zfree;
    strm->opaque = &zalloc_buf;
}

int vmbuf_deflate(struct vmbuf *buf) {
    return vmbuf_deflate3(buf, Z_DEFAULT_COMPRESSION);
}

int vmbuf_deflate3(struct vmbuf *buf, int level) {
    static struct vmbuf outbuf = VMBUF_INITIALIZER;
    vmbuf_init(&outbuf, vmbuf_ravail(buf));
    if (0 > vmbuf_deflate4(buf, &outbuf, level))
        return -1;
    vmbuf_swap(&outbuf, buf);
    return 0;
}


int vmbuf_deflate2(struct vmbuf *inbuf, struct vmbuf *outbuf) {
    return vmbuf_deflate4(inbuf, outbuf, Z_DEFAULT_COMPRESSION);
}

int vmbuf_deflate4(struct vmbuf *inbuf, struct vmbuf *outbuf, int level) {
    z_stream strm;
    _init_alloc(&strm);
    if (Z_OK != deflateInit2(&strm, level, Z_DEFLATED, 15+16, 8, Z_DEFAULT_STRATEGY))
        return -1;
    int flush;
    do {
        strm.next_in = (uint8_t *)vmbuf_rloc(inbuf);
        strm.avail_in = vmbuf_ravail(inbuf);
        vmbuf_resize_if_less(outbuf, strm.avail_in << 1);
        strm.next_out = (uint8_t *)vmbuf_wloc(outbuf);
        strm.avail_out = vmbuf_wavail(outbuf);
        flush = vmbuf_ravail(inbuf) == 0 ? Z_FINISH : Z_NO_FLUSH;
        if (Z_STREAM_ERROR == deflate(&strm, flush)) {
            deflateEnd(&strm);
            return -1;
        }
        vmbuf_wseek(outbuf, vmbuf_wavail(outbuf) - strm.avail_out);
        vmbuf_rseek(inbuf, vmbuf_ravail(inbuf) - strm.avail_in);
    } while (flush != Z_FINISH);
    deflateEnd(&strm);
    return 0;
}

int vmbuf_deflate_ptr(const void *inbuf, size_t inbuf_size, struct vmbuf *outbuf) {
    return vmbuf_deflate_ptr2(inbuf, inbuf_size, outbuf, Z_DEFAULT_COMPRESSION);
}

int vmbuf_deflate_ptr2(const void *inbuf, size_t inbuf_size, struct vmbuf *outbuf, int level) {
    z_stream strm;
    _init_alloc(&strm);
    if (Z_OK != deflateInit2(&strm, level, Z_DEFLATED, 15+16, 8, Z_DEFAULT_STRATEGY))
        return -1;
    int flush;
    strm.next_in = (void *)inbuf;
    strm.avail_in = inbuf_size;
    do {
        vmbuf_resize_if_less(outbuf, strm.avail_in << 1);
        strm.next_out = (uint8_t *)vmbuf_wloc(outbuf);
        strm.avail_out = vmbuf_wavail(outbuf);
        flush = strm.avail_in == 0 ? Z_FINISH : Z_NO_FLUSH;
        if (Z_STREAM_ERROR == deflate(&strm, flush)) {
            deflateEnd(&strm);
            return -1;
        }
        vmbuf_wseek(outbuf, vmbuf_wavail(outbuf) - strm.avail_out);
    } while (flush != Z_FINISH);
    deflateEnd(&strm);
    return 0;
}

int vmbuf_inflate(struct vmbuf *buf) {
    static struct vmbuf outbuf = VMBUF_INITIALIZER;
    vmbuf_init(&outbuf, vmbuf_ravail(buf));
    if (0 > vmbuf_inflate2(buf, &outbuf))
        return -1;
    vmbuf_swap(&outbuf, buf);
    return 0;
}

int vmbuf_inflate2(struct vmbuf *inbuf, struct vmbuf *outbuf) {
    z_stream strm;
    _init_alloc(&strm);
    if (Z_OK != inflateInit2(&strm, 15+16))
        return -1;
    for (;;)
    {
        strm.next_in = (uint8_t *)vmbuf_rloc(inbuf);
        strm.avail_in = vmbuf_ravail(inbuf);
        if (0 == strm.avail_in)
            break;
        vmbuf_resize_if_less(outbuf, strm.avail_in << 1);
        strm.next_out = (uint8_t *)vmbuf_wloc(outbuf);
        strm.avail_out = vmbuf_wavail(outbuf);
        int res = inflate(&strm, Z_NO_FLUSH);
        vmbuf_wseek(outbuf, vmbuf_wavail(outbuf) - strm.avail_out);
        if (res == Z_STREAM_END)
            break;
        if (Z_OK != res)
            return inflateEnd(&strm), -1;
        vmbuf_rseek(inbuf, vmbuf_ravail(inbuf) - strm.avail_in);
    }
    inflateEnd(&strm);
    return 0;
}

int vmbuf_inflate_gzip(void *inbuf, size_t inbuf_size, struct vmbuf *outbuf) {
    return vmbuf_inflate_ptr(inbuf, inbuf_size, outbuf);
}

int vmbuf_inflate_ptr(void *inbuf, size_t inbuf_size, struct vmbuf *outbuf) {
    z_stream strm;
    _init_alloc(&strm);
    if (Z_OK != inflateInit2(&strm, 15+16))
        return -1;
    strm.next_in = inbuf;
    strm.avail_in = inbuf_size;
    for (;0 < strm.avail_in;)
    {
        if (0 > vmbuf_resize_if_less(outbuf, strm.avail_in << 1))
            return inflateEnd(&strm), -1;
        strm.next_out = (uint8_t *)vmbuf_wloc(outbuf);
        strm.avail_out = vmbuf_wavail(outbuf);
        int res = inflate(&strm, Z_NO_FLUSH);
        vmbuf_wseek(outbuf, vmbuf_wavail(outbuf) - strm.avail_out);
        if (res == Z_STREAM_END)
            return inflateEnd(&strm), strm.avail_in == 0 ? 0 : -1; /* return error if inbuf has extra data */
        if (Z_OK != res)
            return inflateEnd(&strm), -1;
    }
    inflateEnd(&strm);
    return -1; /* if we reached here, we have partial outbuf */

}

