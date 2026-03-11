#include "../src/wrm/memory/memory.h"
#include "test.h"
#include <stdlib.h>

//--- SETUP -------------------------------------------------------------------

// setup each test
void before_each()
{
    mem_init(realloc);
}

// shutdown each test
void after_each()
{

}

//--- TESTS -------------------------------------------------------------------

wrm_TESTDEF(test_init)
{
    if(id_counter != 1) { wrm_FAIL(); }
    if(internal_resize != realloc) { wrm_FAIL(); }

    wrm_PASS();
}

wrm_TESTDEF(test_init_pool) 
{
    return true;
}


//--- RUN ---------------------------------------------------------------------

wrm_Test to_run[] = {
    wrm_TEST(test_init_pool)
};

wrm_RUN("MEMORY", to_run, before_each, after_each);


