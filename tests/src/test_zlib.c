#include "ribs.h"
#include <stdio.h>
#include "minunit.h"
#include "ribs_zlib.h"
#include <zlib.h>

#define DATA_SIZE 2913 /* this will fill exactly the wavail space on a fresh vmbuf */

const char *test_zlib_vmbuf() {
    ssize_t data_size;
    for (data_size = DATA_SIZE - 5; data_size < DATA_SIZE + 5; ++data_size) {
        int data[data_size];
        int i;
        for (i = 0; i < data_size; ++i) {
            data[i] = i;
        }
        uLongf expected_size = compressBound(sizeof(data));
        uint8_t *expected = malloc(expected_size);
        compress(expected, &expected_size, (uint8_t *)data, sizeof(data));

        struct vmbuf compressed = VMBUF_INITIALIZER;
        vmbuf_init(&compressed, 128);
        if (0 > vmbuf_deflate_ptr(data, sizeof(data), &compressed))
            mu_fail("vmbuf_deflate_ptr() failed");
        mu_assert_eqi(expected_size + 12 /* gzip header and trailer */, vmbuf_wlocpos(&compressed));
        struct vmbuf decompressed = VMBUF_INITIALIZER;
        vmbuf_init(&decompressed, 128);
        vmbuf_inflate2(&compressed, &decompressed);
        mu_assert_eqi(vmbuf_wlocpos(&decompressed), sizeof(data));
        typeof(data[0]) *decompressed_buf = vmbuf_mem(&decompressed);
        for (i = 0; i < data_size; ++i)
            mu_assert_eqi_idx(i, data[i], decompressed_buf[i]);
        vmbuf_free(&compressed);
        vmbuf_free(&decompressed);
        free(expected);
    }
    return NULL;
}
