
/******************************************************************************
 * Data race assertion (desired)
 ******************************************************************************/

#include <gtest/gtest.h>
#include <thread>

int value = 5;

TEST(DataRaceTest, NoDataRaces)
{
    /* ASSERT_NO_DATA_RACE */
    {
        std::thread t1([]{ value += 5; });
        std::thread t2([]{ value *= 2; });
        
        t1.join();
        t2.join();
    }
}
