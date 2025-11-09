
/******************************************************************************
 * Programmatic data race detection
 ******************************************************************************/

TEST(DataRaceTest, NoDataRaces)
{
    bool passed = runAndSearchForDataRaceInForkedProcess([]
    {
        std::thread t1([]{ value += 5; });
        std::thread t2([]{ value *= 2; });
        t1.join();
        t2.join();
    });

    ASSERT_TRUE(passed);
}
