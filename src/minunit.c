#include "minunit.h"
#include "vmbuf.h"
#include <stdio.h>

int mu_tests_run = 0;

static struct vmbuf msgbuf = VMBUF_INITIALIZER;

const char *mu_assert_msg(const char *filename, unsigned int lineno, int64_t index, const char *message, ...) {
    vmbuf_init(&msgbuf, 4096);
    if (INT64_MIN != index)
        vmbuf_sprintf(&msgbuf, "%s@%u  [%" PRId64 "]: ", filename, lineno, index);
    else
        vmbuf_sprintf(&msgbuf, "%s@%u: ", filename, lineno);
    va_list ap;
    va_start(ap, message);
    if (0 > vmbuf_vsprintf(&msgbuf, message, ap))
        return "failed to construct msg";
    va_end(ap);
    return vmbuf_data(&msgbuf);
}

int mu_run_tests_wrapper(const char *(*all_tests)()) {
     const char *result = all_tests();
     if (result != 0) {
         printf("FAILED: %s\n", result);
     }
     else {
         printf("ALL TESTS PASSED\n");
     }
     printf("Tests run: %d\n", mu_tests_run);
     return result != 0;
 }

