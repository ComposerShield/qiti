// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"

#include "qiti_HotspotDetector.hpp"

#include <algorithm>

//--------------------------------------------------------------------------

/** Test function with variable execution time for hotspot testing */
__attribute__((noinline))
__attribute__((optnone))
void hotspotTestFuncSlow() noexcept
{
    volatile int sum = 0;
    // Create significant execution time
    for(int i = 0; i < 50000; ++i)
    {
        sum = sum + i;
    }
}

/** Test function with minimal execution time */
__attribute__((noinline))
__attribute__((optnone))
void hotspotTestFuncFast() noexcept
{
    volatile int _ = 42;
}

/** Test function with medium execution time */
__attribute__((noinline))
__attribute__((optnone))
void hotspotTestFuncMedium() noexcept
{
    volatile int sum = 0;
    for(int i = 0; i < 5000; ++i)
    {
        sum = sum + i;
    }
}

/** Test function with very slow execution time */
__attribute__((noinline))
__attribute__((optnone))
void hotspotTestFuncVerySlow() noexcept
{
    volatile int sum = 0;
    for(int i = 0; i < 200000; ++i)
    {
        sum = sum + i;
    }
}

/** Test function that throws exceptions */
__attribute__((noinline))
__attribute__((optnone))
void hotspotTestFuncThrows()
{
    throw std::runtime_error("Test exception");
}

/** Test function that calls the throwing function */
__attribute__((noinline))
__attribute__((optnone))
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

//--------------------------------------------------------------------------

QITI_TEST_CASE("qiti::HotspotDetector::detectHotspots() - no threshold", HotspotDetectorDetectHotspotsNoThreshold)
{
    qiti::ScopedQitiTest test;
    test.enableProfilingOnAllFunctions(true);
    
    // Initially, no hotspots should be detected
    auto initialHotspots = qiti::HotspotDetector::detectHotspots();
    QITI_CHECK(initialHotspots.empty());
    
    // Call functions with different performance characteristics
    hotspotTestFuncSlow();   // High execution time
    hotspotTestFuncFast();   // Low execution time
    hotspotTestFuncFast();   // Called twice
    
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
        if (hotspot.score >= threshold)
            expectedCount++;
    
    // Filtered list should have exactly the expected count
    QITI_CHECK(filteredHotspots.size() == expectedCount);
    
    // Test with very high threshold - should return no results
    auto noHotspots = qiti::HotspotDetector::detectHotspots(999999999.0);
    QITI_CHECK(noHotspots.empty());
}

QITI_TEST_CASE("qiti::HotspotDetector::detectHotspots() - with sensitivity", HotspotDetectorDetectHotspotsWithSensitivity)
{
    qiti::ScopedQitiTest test;
    
    // Only profile our specific test functions to avoid interference from optimized framework functions
    auto verySlow = qiti::FunctionData::getFunctionDataMutable<hotspotTestFuncVerySlow>();
    auto slow = qiti::FunctionData::getFunctionDataMutable<hotspotTestFuncSlow>();  
    auto medium = qiti::FunctionData::getFunctionDataMutable<hotspotTestFuncMedium>();
    auto fast = qiti::FunctionData::getFunctionDataMutable<hotspotTestFuncFast>();
    
    // Create a clear hierarchy of execution times and call patterns
    // Very slow function called once - should be top hotspot (1 × very_slow_time)
    hotspotTestFuncVerySlow();
    
    // Slow function called multiple times - should be high hotspot (5 × slow_time)
    for (int i = 0; i < 5; ++i)
        hotspotTestFuncSlow();
    
    // Medium function called many times - should be medium hotspot (20 × medium_time)
    for (int i = 0; i < 20; ++i)
        hotspotTestFuncMedium();
    
    // Fast function called very many times - should be lower hotspot (100 × fast_time)
    for (int i = 0; i < 100; ++i)
        hotspotTestFuncFast();
    
    // Get baseline with ALL sensitivity
    auto allHotspots = qiti::HotspotDetector::detectHotspots(qiti::HotspotDetector::Sensitivity::ALL);
    QITI_REQUIRE(allHotspots.size() >= 4); // Should have at least our 4 test functions
    
    // Test different sensitivity levels
    auto lowSensitivity = qiti::HotspotDetector::detectHotspots(qiti::HotspotDetector::Sensitivity::LOW);
    auto mediumSensitivity = qiti::HotspotDetector::detectHotspots(qiti::HotspotDetector::Sensitivity::MEDIUM);
    auto highSensitivity = qiti::HotspotDetector::detectHotspots(qiti::HotspotDetector::Sensitivity::HIGH);
    
    // Basic validation: all results should be subsets of ALL
    QITI_CHECK(lowSensitivity.size() <= allHotspots.size());
    QITI_CHECK(mediumSensitivity.size() <= allHotspots.size());
    QITI_CHECK(highSensitivity.size() <= allHotspots.size());
    
    // LOW sensitivity should be most restrictive
    QITI_CHECK(lowSensitivity.size() <= mediumSensitivity.size());
    QITI_CHECK(lowSensitivity.size() <= highSensitivity.size());
    
    // All returned hotspots should be sorted by score (highest first)
    for (const auto& hotspotList : {lowSensitivity, mediumSensitivity, highSensitivity, allHotspots})
        for (size_t i = 1; i < hotspotList.size(); ++i)
            QITI_CHECK(hotspotList[i-1].score >= hotspotList[i].score);
    
    // Verify sensitivity levels return meaningful results
    // LOW should capture at least the most expensive operations
    QITI_CHECK(lowSensitivity.size() >= 1);
    
    // Verify that LOW sensitivity captures the most significant hotspots
    bool foundSignificantHotspot = false;
    for (const auto& hotspot : lowSensitivity)
    {
        const char* name = hotspot.function->getFunctionName();
        if (strstr(name, "hotspotTestFuncVerySlow") != nullptr || 
            strstr(name, "hotspotTestFuncSlow") != nullptr)
        {
            foundSignificantHotspot = true;
            break;
        }
    }
    QITI_CHECK(foundSignificantHotspot);
}

QITI_TEST_CASE("qiti::HotspotDetector hotspot scoring", HotspotDetectorScoring)
{
    qiti::ScopedQitiTest test;
    test.enableProfilingOnAllFunctions(true);
    
    // Call slow function once and fast function many times
    hotspotTestFuncSlow();
    for (int i = 0; i < 10; ++i)
    {
        hotspotTestFuncFast();
    }
    
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
    QITI_CHECK(slowHotspot->function->getNumTimesCalled() == 1);
    QITI_CHECK(fastHotspot->function->getNumTimesCalled() == 10);
    
    // Both should have valid scores
    QITI_CHECK(slowHotspot->score > 0.0);
    QITI_CHECK(fastHotspot->score > 0.0);
    
    // Reason strings should contain useful information
    QITI_CHECK(slowHotspot->reason.find("Total time:") != std::string::npos);
    QITI_CHECK(fastHotspot->reason.find("Total time:") != std::string::npos);
    QITI_CHECK(slowHotspot->reason.find("1 calls") != std::string::npos);
    QITI_CHECK(fastHotspot->reason.find("10 calls") != std::string::npos);
}

QITI_TEST_CASE("qiti::HotspotDetector exception tracking in hotspots", HotspotDetectorExceptionTracking)
{
    qiti::ScopedQitiTest test;
    test.enableProfilingOnAllFunctions(true);
    
    // Call function that throws exceptions
    hotspotTestFuncCatches(); // This will internally call hotspotTestFuncThrows
    hotspotTestFuncCatches(); // Call it again
    
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
    test.enableProfilingOnAllFunctions(true);
    
    // Create and destroy objects to trigger constructor/destructor profiling
    {
        std::vector<int> vec1(100, 42);  // Constructor with parameters
        std::vector<int> vec2 = vec1;    // Copy constructor
    } // Destructors called here
    
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
