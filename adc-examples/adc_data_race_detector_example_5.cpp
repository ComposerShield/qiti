
/******************************************************************************
 * Programmatic data race detection
 ******************************************************************************/

#include <gtest/gtest.h>
#include <thread>
#include <unistd.h>
#include <sys/wait.h>
#include <fstream>
#include <string>

int value = 5;

bool runAndSearchForDataRaceInForkedProcess(auto lambda)
{
    pid_t pid = fork();

    if (pid == 0)  // Child process
    {
        lambda();
        exit(0);
    }

    // Parent process: wait for child
    int status;
    waitpid(pid, &status, 0);

    // Check TSan log for "data race"
    std::ifstream log("/tmp/tsan_report.txt");
    std::string line;
    while (std::getline(log, line))
        if (line.find("data race") != std::string::npos)
            return false;  // Data race detected

    return true;  // No data race
}

TEST(DataRaceTest, NoDataRaces)
{
    bool passed = runAndSearchForDataRaceInForkedProcess([]
    {
        std::thread t1([]{ value += 5; });
        std::thread t2([]{ value *= 2; });
        t1.join();
        t2.join();
    });

    ASSERT_TRUE(passed) << "Data race detected!";
}
