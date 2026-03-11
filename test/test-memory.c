#include "../src/wrm/memory/memory.h"
#include "test.h"

wrm_UNIT(test_init) 
{
    return true;
}

wrm_UNIT(test_init_pool) 
{
    return true;
}


wrm_Test tests[] = {
    wrm_TEST(test_init_pool)
};

wrm_RUN("MEMORY", tests);


