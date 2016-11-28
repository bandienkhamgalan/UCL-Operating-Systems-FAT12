#include "greatest/greatest.h"
#include "ClusterChainTest.h"
#include "FATImageTest.h"
#include "HelpersTest.h"

#define NONE 0
#define INFO 1
#define DETAIL 2
#define DEBUG 3
int log_level = NONE;

GREATEST_MAIN_DEFS();

int main(int argc, char** argv)
{
    GREATEST_MAIN_BEGIN();
    
    RUN_SUITE(ClusterChainTest);
    RUN_SUITE(FATImageTest);
    RUN_SUITE(HelpersTest);

    GREATEST_MAIN_END();
}