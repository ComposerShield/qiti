
/******************************************************************************
 * Data race example
 ******************************************************************************/

#include <iostream>
#include <thread>

int value = 5;

TEST(DataRaceTest, NoDataRaces)
{
    std::thread t1([]{ value += 5 });
    std::thread t2([]{ value *= 2 });

    t1.join();
    t2.join();

    // Undefined behavior
    // value is indeterminate
}
