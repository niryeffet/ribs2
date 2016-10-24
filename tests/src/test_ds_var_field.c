#include "test_ds_var_field.h"
#include "ribs.h"
#include "minunit.h"

#define WRITE_ARRAY(x, ...) \
    uint32_t arr ## x[] = { __VA_ARGS__ }; \
    ds_var_field_writer_write(&vfw, arr ## x, sizeof(arr ## x))

#define CHECK_ARRAY(x) \
    { size_t n = 0; \
    uint32_t *a = ds_var_field_get_array(&vf, x-1, &n); \
    mu_assert_eqi(n, sizeof(arr ## x)/sizeof(arr ## x[0])); \
    size_t i; \
    for (i = 0; i < n; ++i) \
        mu_assert_eqi_idx(i, a[i], arr ## x[i]); }

const char *test_ds_var_field() {
    const char *filename = "/tmp/ribs2_vf_arr_test";
    struct ds_var_field_writer vfw = DS_VAR_FIELD_WRITER_INITIALIZER;
    if (0 > ds_var_field_writer_init_array(&vfw, filename, ds_type_uint32_t))
        mu_fail("ds_var_field_writer_init_array: %s", filename)

    /* data */
    WRITE_ARRAY(1, 10, 20, 30 );
    WRITE_ARRAY(2, 100, 200, 300 );
    WRITE_ARRAY(3, 123 );
    WRITE_ARRAY(4, 123, 456, 0, 0, 0, 0, 0, 0, 0, 123456, 0, 0, 123 );
    WRITE_ARRAY(5);

    ds_var_field_writer_close(&vfw);

    struct ds_var_field vf = DS_VAR_FIELD_INITIALIZER;
    if (0 > ds_var_field_init(&vf, filename))
        mu_fail("ds_var_field_init: %s", filename);
    mu_assert_eqi(ds_var_field_num_elements(&vf), 5);

    CHECK_ARRAY(1);
    CHECK_ARRAY(2);
    CHECK_ARRAY(3);
    CHECK_ARRAY(4);
    CHECK_ARRAY(5);
    if (0 > ds_var_field_free(&vf))
        mu_fail("ds_var_field_free: %s", filename);
    unlink(filename);
    return NULL;
}
