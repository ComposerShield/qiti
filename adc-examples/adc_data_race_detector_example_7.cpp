
/******************************************************************************
 * Data race detection in Qiti
 ******************************************************************************/

#include "qiti_include.hpp"
#include <gtest/gtest.h>
#include <thread>

int value = 5;

#ifdef QITI_ENABLE_CLANG_THREAD_SANITIZER
TEST(DataRaceTest, NoDataRaces)
{
    qiti::ScopedQitiTest test;

    auto detector = qiti::ThreadSanitizer::createDataRaceDetector();

    detector->run([]
    {
        std::thread t1([]{ value += 5; });
        std::thread t2([]{ value *= 2; });
        t1.join();
        t2.join();
    });

    ASSERT_TRUE(detector->passed());
}
#endif
