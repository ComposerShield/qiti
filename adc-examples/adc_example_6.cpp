
/******************************************************************************
 * Detect unwanted heap allocations within test suite
 ******************************************************************************/

#include <gtest/gtest.h>
#include "MyPluginProcessor.h"

std::size_t getNumHeapAllocations();

TEST(MemoryTest, NoHeapAllocationsInProcessBlock)
{
    std::size_t allocationsBeforeTest = getNumHeapAllocations();
    
    MyPluginProcessor processor;
    processor.processBlock(/*Audio stuff here*/);
    
    std::size_t allocationsAfterTest = getNumHeapAllocations();
    
    EXPECT_EQ(allocationsBeforeTest, allocationsAfterTest);
}
