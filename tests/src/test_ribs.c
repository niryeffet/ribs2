#include "minunit.h"
#include "test_kmeans.h"

static const char *all_tests() {
    mu_run_test(test_kmeans);
    return 0;
}

int main() {
    return mu_run_tests_wrapper(all_tests);
}