
/******************************************************************************
 * Using qiti to track function calls
 ******************************************************************************/

#include <gtest/gtest.h>
#include <qiti_include.hpp>

void processAudio();

TEST(ProfileTest, ProcessAudioCalledTwice)
{
    qiti::ScopedQitiTest test;

    auto funcData = qiti::FunctionData::getFunctionData<&processAudio>();

    processAudio();
    processAudio();

    EXPECT_EQ(funcData->getNumTimesCalled(), 2);
}
