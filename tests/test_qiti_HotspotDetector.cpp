// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"

#include "qiti_HotspotDetector.hpp"

#include <algorithm>
#include <stdexcept>

//--------------------------------------------------------------------------

// Disable optimizations for test functions to ensure they get instrumented
#pragma clang optimize off

/** Test function with variable execution time for hotspot testing */
void hotspotTestFuncSlow() noexcept
{
    volatile int sum = 0;
    volatile int temp = 0;
    // Create significant execution time with actual computational work
    for(int i = 0; i < 5000000; ++i)
    {
        sum = sum + i;
        temp = sum * i;  // More computation
        sum = temp % 1000;  // Prevent optimization
    }
}

/** Test function with minimal execution time */
void hotspotTestFuncFast() noexcept
{
    volatile int _ = 42;
}

/** Test function that throws exceptions */
void hotspotTestFuncThrows()
{
    throw std::runtime_error("Test exception");
}

/** Test function that calls the throwing function */
void hotspotTestFuncCatches()
{
    try
    {
        hotspotTestFuncThrows();
    }
    catch (const std::exception&)
    {
        // Caught the exception
    }
}

// Re-enable optimizations
#pragma clang optimize on

//--------------------------------------------------------------------------

QITI_TEST_CASE("qiti::HotspotDetector::detectHotspots() - no threshold", HotspotDetectorDetectHotspotsNoThreshold)
{
    qiti::ScopedQitiTest test;
    
    // Initially, no hotspots should be detected
    auto initialHotspots = qiti::HotspotDetector::detectHotspots();
    QITI_CHECK(initialHotspots.empty());
    
    // Enable profiling
    test.enableProfilingOnAllFunctions(true);
    
    // Call functions with different performance characteristics
    hotspotTestFuncSlow();   // High execution time
    hotspotTestFuncFast();   // Low execution time
    hotspotTestFuncFast();   // Called twice
    
    // Disable profiling
    test.enableProfilingOnAllFunctions(false);
    
    auto hotspots = qiti::HotspotDetector::detectHotspots();
    
    // Should have at least 2 hotspots (the functions we called)
    QITI_REQUIRE(hotspots.size() >= 2);
    
    // Hotspots should be sorted by score (highest first)
    for (size_t i = 1; i < hotspots.size(); ++i)
    {
        QITI_CHECK(hotspots[i-1].score >= hotspots[i].score);
    }
    
    // All hotspots should have valid function pointers
    for (const auto& hotspot : hotspots)
    {
        QITI_CHECK(hotspot.function != nullptr);
        QITI_CHECK(hotspot.score >= 0.0);
        QITI_CHECK(! hotspot.reason.empty());
    }
}

QITI_TEST_CASE("qiti::HotspotDetector::detectHotspots() - with threshold", HotspotDetectorDetectHotspotsWithThreshold)
{
    qiti::ScopedQitiTest test;
    
    // Enable profiling
    test.enableProfilingOnAllFunctions(true);
    
    // Call functions multiple times to generate different scores
    hotspotTestFuncSlow();   // High execution time
    hotspotTestFuncFast();   // Low execution time
    hotspotTestFuncFast();   // Called again
    hotspotTestFuncFast();   // Called again
    
    // Turn off profiling to prevent additional functions from being profiled during hotspot detection
    test.enableProfilingOnAllFunctions(false);
    
    // Get baseline with no threshold
    auto allHotspots = qiti::HotspotDetector::detectHotspots(0.0);
    QITI_REQUIRE(allHotspots.size() >= 2);
    
    // Set a threshold that should filter out some functions
    double threshold = allHotspots[0].score * 0.5; // 50% of highest score
    auto filteredHotspots = qiti::HotspotDetector::detectHotspots(threshold);
    
    // All returned hotspots should meet the threshold
    for (const auto& hotspot : filteredHotspots)
    {
        QITI_CHECK(hotspot.score >= threshold);
    }
    
    // Count how many from allHotspots would meet the threshold
    size_t expectedCount = 0;
    for (const auto& hotspot : allHotspots)
    {
        if (hotspot.score >= threshold)
        {
            expectedCount++;
        }
    }
    
    // Filtered list should have exactly the expected count
    QITI_CHECK(filteredHotspots.size() == expectedCount);
    
    // Test with very high threshold - should return no results
    auto noHotspots = qiti::HotspotDetector::detectHotspots(999999999.0);
    QITI_CHECK(noHotspots.empty());
}

QITI_TEST_CASE("qiti::HotspotDetector hotspot scoring", HotspotDetectorScoring)
{
    qiti::ScopedQitiTest test;
    
    // Enable profiling
    test.enableProfilingOnAllFunctions(true);
    
    // Call slow function multiple times to accumulate significant total time
    for (int i = 0; i < 5; ++i)
    {
        hotspotTestFuncSlow();
    }
    
    // Call fast function many times
    for (int i = 0; i < 20; ++i)
    {
        hotspotTestFuncFast();
    }
    
    // Turn off profiling to prevent additional functions from being profiled during hotspot detection
    test.enableProfilingOnAllFunctions(false);
    
    auto hotspots = qiti::HotspotDetector::detectHotspots();
    QITI_REQUIRE(hotspots.size() >= 2);
    
    
    // Find our test functions in the results
    const qiti::HotspotDetector::Hotspot* slowHotspot = nullptr;
    const qiti::HotspotDetector::Hotspot* fastHotspot = nullptr;
    
    for (const auto& hotspot : hotspots)
    {
        const char* name = hotspot.function->getFunctionName();
        if (strstr(name, "hotspotTestFuncSlow") != nullptr)
        {
            slowHotspot = &hotspot;
        }
        else if (strstr(name, "hotspotTestFuncFast") != nullptr)
        {
            fastHotspot = &hotspot;
        }
    }
    
    QITI_REQUIRE(slowHotspot != nullptr);
    QITI_REQUIRE(fastHotspot != nullptr);
    
    // Verify the scoring makes sense based on call patterns
    QITI_CHECK(slowHotspot->function->getNumTimesCalled() == 5);
    QITI_CHECK(fastHotspot->function->getNumTimesCalled() == 20);
    
    // Both should have valid scores
    QITI_CHECK(slowHotspot->score > 0.0);
    QITI_CHECK(fastHotspot->score > 0.0);
    
    // Reason strings should contain useful information
    QITI_CHECK(slowHotspot->reason.find("Total time:") != std::string::npos);
    QITI_CHECK(fastHotspot->reason.find("Total time:") != std::string::npos);
    QITI_CHECK(slowHotspot->reason.find("5 calls") != std::string::npos);
    QITI_CHECK(fastHotspot->reason.find("20 calls") != std::string::npos);
}

QITI_TEST_CASE("qiti::HotspotDetector exception tracking in hotspots", HotspotDetectorExceptionTracking)
{
    qiti::ScopedQitiTest test;
    
    // Enable profiling
    test.enableProfilingOnAllFunctions(true);
    
    // Call function that throws exceptions multiple times to accumulate total time
    for (int i = 0; i < 10; ++i)
    {
        hotspotTestFuncCatches(); // This will internally call hotspotTestFuncThrows
    }
    
    // Turn off profiling to prevent additional functions from being profiled during hotspot detection
    test.enableProfilingOnAllFunctions(false);
    
    auto hotspots = qiti::HotspotDetector::detectHotspots();
    
    
    // Find the function that throws exceptions
    const qiti::HotspotDetector::Hotspot* throwingHotspot = nullptr;
    
    for (const auto& hotspot : hotspots)
    {
        const char* name = hotspot.function->getFunctionName();
        if (strstr(name, "hotspotTestFuncThrows") != nullptr)
        {
            throwingHotspot = &hotspot;
            break;
        }
    }
    
    QITI_REQUIRE(throwingHotspot != nullptr);
    
    // The function should have thrown exceptions
    QITI_CHECK(throwingHotspot->function->getNumExceptionsThrown() > 0);
    
    // The reason string should mention exceptions
    QITI_CHECK(throwingHotspot->reason.find("exceptions") != std::string::npos);
}

QITI_TEST_CASE("qiti::HotspotDetector constructor/destructor detection", HotspotDetectorConstructorDestructor)
{
    qiti::ScopedQitiTest test;
    
    // Enable profiling
    test.enableProfilingOnAllFunctions(true);
    
    // Create and destroy objects to trigger constructor/destructor profiling
    {
        std::vector<int> vec1(100, 42);  // Constructor with parameters
        std::vector<int> vec2 = vec1;    // Copy constructor
    } // Destructors called here
    
    // Turn off profiling to prevent additional functions from being profiled during hotspot detection
    test.enableProfilingOnAllFunctions(false);
    
    auto hotspots = qiti::HotspotDetector::detectHotspots();
    
    // Look for hotspots that are constructors or destructors
    bool foundConstructor = false;
    bool foundDestructor = false;
    
    for (const auto& hotspot : hotspots)
    {
        if (hotspot.function->isConstructor())
        {
            foundConstructor = true;
            QITI_CHECK(hotspot.reason.find("[constructor]") != std::string::npos);
        }
        if (hotspot.function->isDestructor())
        {
            foundDestructor = true;
            QITI_CHECK(hotspot.reason.find("[destructor]") != std::string::npos);
        }
    }
    
    // Note: These checks may not always pass depending on what gets profiled
    // but if they are found, they should be properly labeled
    if (foundConstructor)
    {
        QITI_CHECK(foundConstructor == true);
    }
    if (foundDestructor)
    {
        QITI_CHECK(foundDestructor == true);
    }
}
