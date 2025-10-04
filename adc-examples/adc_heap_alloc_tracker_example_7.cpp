
/******************************************************************************
 * Detect memory leaks within test suite
 ******************************************************************************/

#include <gtest/gtest.h>

std::size_t getBytesCurrentlyAllocated();

void functionWhichShouldNotLeak();

TEST(MemoryTest, NoMemoryLeaks)
{
    auto numBytesBeforeTest = getBytesCurrentlyAllocated();
    
    functionWhichShouldNotLeak();
    
    auto numBytesAfterTest = getBytesCurrentlyAllocated();
    
    EXPECT_EQ(numBytesBeforeTest, numBytesAfterTest);
}
