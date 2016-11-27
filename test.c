#include "greatest/greatest.h"
#include "ClusterChainTest.h"
#include "FATImageTest.h"
#include "HelpersTest.h"

GREATEST_MAIN_DEFS();

int main(int argc, char** argv)
{
    GREATEST_MAIN_BEGIN();
    
    RUN_SUITE(ClusterChainTest);
    RUN_SUITE(FATImageTest);
    RUN_SUITE(HelpersTest);

    GREATEST_MAIN_END();
}