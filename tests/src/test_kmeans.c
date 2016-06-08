#include "ribs.h"
#include "kmeans.h"
#include <stdio.h>
#include "minunit.h"

static double POINT1[] = {1,0,0};
static double POINT2[] = {0,0,2};
static double POINT3[] = {2,0,0};
static double POINT4[] = {3,0,0};
static double POINT5[] = {0,0,2};
static double POINT6[] = {0,0,2};

static double *DATA[] = {
    POINT1,
    POINT2,
    POINT3,
    POINT4,
    POINT5,
    POINT6,
};

static int expected[] = { 1, 0, 1, 1, 0, 0 };

const char *test_kmeans() {
    size_t n = sizeof(DATA)/sizeof(DATA[0]);
    size_t m = sizeof(POINT1)/sizeof(POINT1[0]);
    mu_assert_eqi(n, (sizeof(expected) / sizeof(expected[0])));
    int *labels = k_means(DATA, n, m, 2, 0.0001, NULL);
    size_t i;
    for (i = 0; i < n; ++i) {
        // printf("%d\n", labels[i]);
        mu_assert_eqi_idx(i, labels[i], expected[i]);
        // mu_assert(labels[i] == expected[i], "labels[%zu](%d) != expected[%zu](%d)", i, labels[i], i, expected[i]);
    }
    return NULL;
}