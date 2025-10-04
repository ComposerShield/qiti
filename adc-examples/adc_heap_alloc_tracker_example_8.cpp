
/******************************************************************************
 * Using qiti to detect heap allocations
 ******************************************************************************/

#include <gtest/gtest.h>
#include <qiti_include.hpp>
#include "MyPluginProcessor.h"

TEST(MemoryTest, NoHeapAllocationsInProcessBlock)
{
    qiti::ScopedQitiTest test;
    
    auto funcData = qiti::FunctionData::getFunctionData<&MyPluginProcessor::processBlock>();
    
    MyPluginProcessor processor;
    processor.processBlock(/*Audio stuff here*/);
    
    auto lastCall = funcData->getLastFunctionCall();
    EXPECT_EQ(lastCall.getNumHeapAllocations(), 0);
}
