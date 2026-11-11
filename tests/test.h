/* test.h — мінімальний тестовий фреймворк. */
#ifndef TEST_H
#define TEST_H

#include <stdio.h>

static int tests_run    = 0;
static int tests_failed = 0;

#define CHECK(cond)                                                       \
    do {                                                                  \
        tests_run++;                                                      \
        if (!(cond)) {                                                    \
            tests_failed++;                                               \
            fprintf(stderr, "  FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        }                                                                 \
    } while (0)

#define TEST_SUMMARY(name)                                                \
    (printf("%s: %d/%d passed\n", (name), tests_run - tests_failed, tests_run), \
     tests_failed ? 1 : 0)

#endif
