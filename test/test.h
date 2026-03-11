#ifndef WRM_TEST
#define WRM_TEST
/*--- DESCRIPTION ---------------------------------------------------------------
File test.h

Created Mar 9, 2026
by William R Mungas (wrm)

(last modified Mar 9, 2026)

Description:
Provides macros for running a set of unit tests in a single .c file
-------------------------------------------------------------------------------*/

#include <stdbool.h>

typedef struct wrm_Test {
    char *name;
    bool (*test)(void);
} wrm_Test;

#define wrm_TESTDEF(name) bool name()

#define wrm_TEST(name) (wrm_Test){ ### name, name}

#define wrm_FAIL() printf("Failed on line %d", __LINE__); return false

#define wrm_PASS() printf("Passed!"); return true

#define wrm_RUN(suite, tests, pre_each, post_each) int main(int argc, char **argv) \
{\
    printf("%s:\n", suite);\
    int passing = 0;\
    for(int i = 0; i < sizeof(tests); i++) {\
        pre_each();\
        printf(" [%s]: ", tests[i].name);\
        if(tests[i].test()) { passing++; }\
        post_each();\
    }\
    printf("%f%% passing (%d / %d total) \n", (float) passing / (float) sizeof(tests), passing, sizeof(tests));\
    exit(passing == sizeof(tests) ? 0 : 1);\
}

#endif // end include guards