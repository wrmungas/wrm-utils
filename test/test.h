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

#define wrm_TEST(name) (wrm_Test){ ### name, name}

#define wrm_RUN(suite, tests) int main(int argc, char **argv) \
{\
    printf("%s:\n", suite);\
    int passing = 0;\
    for(int i = 0; i < sizeof(tests); i++) {\
        printf(" [%s]: ", tests[i].name);\
        bool result = tests[i].test();\
        if(result) { passing++; }\
        printf("%s\n", result ? "Success" : "Failure");\
    }\
    printf("%d / %d total passing\n");\
    exit(passing == sizeof(tests) ? 0 : 1);\
}

#define wrm_UNIT(name) bool name(void)

#endif // end include guards