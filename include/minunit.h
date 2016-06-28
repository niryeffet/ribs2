#ifndef _MINUNIT__H_
#define _MINUNIT__H_

/*
 *  Inspired by: http://www.jera.com/techinfo/jtns/jtn002.html
 */
#include <inttypes.h>

#define mu_fail_idx(index, message, ...) return mu_assert_msg(__FILE__, __LINE__, index, message, ##__VA_ARGS__);
#define mu_fail(message, ...) mu_fail_idx(INT64_MIN, message, ##__VA_ARGS__)

#define mu_assert_idx(index, test, message, ...) do { if (!(test)) return mu_assert_msg(__FILE__, __LINE__, index, message, ##__VA_ARGS__); } while (0)
#define mu_assert(test, message, ...) mu_assert_idx(INT64_MIN, test, message, ##__VA_ARGS__)
#define mu_assert_eq(actual, expected) mu_assert((actual) == (expected), #actual " != " #expected)

#define mu_assert_eq2_idx(index, test, actual, expected, fmt, type) mu_assert_idx(index, test, #actual " != " #expected "    actual: " fmt ", expected: " fmt, (type)(actual), (type)(expected))
#define mu_assert_eqi_idx(index, actual, expected) mu_assert_eq2_idx(index, (actual) == (expected), actual, expected, "%" PRId64, int64_t)
#define mu_assert_eqf_idx(index, actual, expected) mu_assert_eq2_idx(index, fabs(actual - expected) < DBL_EPSILON, actual, expected, "%.5f", double)
#define mu_assert_eqs_idx(index, actual, expected) mu_assert_eq2_idx(index, 0 == strcmp(actual, expected), actual, expected, "\"%s\"", const char *)

#define mu_assert_eq2(test, actual, expected, fmt, type) mu_assert_eq2_idx(INT64_MIN, actual, expected, fmt, type)
#define mu_assert_eqi(actual, expected) mu_assert_eqi_idx(INT64_MIN, actual, expected)
#define mu_assert_eqf(actual, expected) mu_assert_eqf_idx(INT64_MIN, actual, expected)
#define mu_assert_eqs(actual, expected) mu_assert_eqs_idx(INT64_MIN, actual, expected)

#define mu_run_test(test) do { const char *message = test(); ++mu_tests_run; \
                            if (message) return message; } while (0)
extern int mu_tests_run;

int mu_run_tests_wrapper(const char *(*all_tests)());

/*
 * used internally
 */
const char *mu_assert_msg(const char *filename, unsigned int lineno, int64_t index, const char *message, ...) __attribute__ ((format (gnu_printf, 4, 5)));


#endif /* _MINUNIT__H_ */