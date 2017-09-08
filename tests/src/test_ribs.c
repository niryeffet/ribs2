#include "minunit.h"
#include "test_kmeans.h"
#include "test_ds_var_field.h"
#ifdef HAVE_ZLIB
#include "test_zlib.h"
#endif

static const char *all_tests() {
    mu_run_test(test_kmeans);
    mu_run_test(test_ds_var_field);
#ifdef HAVE_ZLIB
    mu_run_test(test_zlib_vmbuf);
#endif
    return 0;
}

int main() {
    return mu_run_tests_wrapper(all_tests);
}
