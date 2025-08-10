
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"

#include "qiti_LeakSanitizer.hpp"

#include <utility> // std::move

// Disable optimizations to prevent compiler from eliminating intentional memory leaks in tests
#pragma clang optimize off

//--------------------------------------------------------------------------

#if __APPLE__ // LSAN only supported on MacOS, TODO: support Linux
QITI_TEST_CASE("qiti::LeakSanitizer::defaultConstructor", LeakSanitizerDefaultConstructor)
{
    qiti::ScopedQitiTest test;
    
    qiti::LeakSanitizer lsan;
    QITI_REQUIRE(lsan.passed());
    QITI_REQUIRE(! lsan.failed());
}

QITI_TEST_CASE("qiti::LeakSanitizer::noLeakDetection", LeakSanitizerNoLeakDetection)
{
    qiti::ScopedQitiTest test;
    
    qiti::LeakSanitizer lsan;
    lsan.run([]()
    {
        // Code that doesn't allocate anything
        int x = 42;
        (void)x;
    });
    QITI_REQUIRE(lsan.passed());
    QITI_REQUIRE(! lsan.failed());
}

QITI_TEST_CASE("qiti::LeakSanitizer::properAllocationDeallocation", LeakSanitizerProperAllocationDeallocation)
{
    qiti::ScopedQitiTest test;
    
    qiti::LeakSanitizer lsan;
    lsan.run([]()
    {
        int* ptr = new int(42);
        QITI_REQUIRE(*ptr == 42);
        delete ptr;
    });
    QITI_REQUIRE(lsan.passed());
}

QITI_TEST_CASE("qiti::LeakSanitizer::arrayAllocationDeallocation", LeakSanitizerArrayAllocationDeallocation)
{
    qiti::ScopedQitiTest test;
    
    qiti::LeakSanitizer lsan;
    lsan.run([]()
    {
        int* arr = new int[10];
        arr[0] = 1;
        arr[9] = 10;
        delete[] arr;
    });
    QITI_REQUIRE(lsan.passed());
}

QITI_TEST_CASE("qiti::LeakSanitizer::memoryLeak", LeakSanitizerMemoryLeak)
{
    qiti::ScopedQitiTest test;
    
    qiti::LeakSanitizer lsan;
    lsan.run([]()
    {
        int* ptr = new int(42);
        // Intentionally don't delete ptr - this should be detected as a leak
        (void)ptr;
    });
    QITI_REQUIRE(lsan.failed());
    QITI_REQUIRE(! lsan.passed());
}

QITI_TEST_CASE("qiti::LeakSanitizer::arrayMemoryLeak", LeakSanitizerArrayMemoryLeak)
{
    qiti::ScopedQitiTest test;
    
    qiti::LeakSanitizer lsan;
    lsan.run([]()
    {
        int* arr = new int[100];
        arr[0] = 1;
        // Intentionally don't delete[] arr - this should be detected as a leak
    });
    QITI_REQUIRE(lsan.failed());
}

QITI_TEST_CASE("qiti::LeakSanitizer::nullptrFunction", LeakSanitizerNullptrFunction)
{
    qiti::ScopedQitiTest test;
    
    qiti::LeakSanitizer lsan;
    lsan.run(nullptr);
    QITI_REQUIRE(lsan.passed());
}

QITI_TEST_CASE("qiti::LeakSanitizer::multipleRuns", LeakSanitizerMultipleRuns)
{
    qiti::ScopedQitiTest test;
    
    qiti::LeakSanitizer lsan;
    
    // First run - no leak
    lsan.run([]()
    {
        int* ptr = new int(1);
        delete ptr;
    });
    QITI_REQUIRE(lsan.passed());
    
    // Second run - with leak (should fail the whole test)
    lsan.run([]()
    {
        int* ptr = new int(2);
        (void)ptr; // leak
    });
    QITI_REQUIRE(lsan.failed());
    
    // Third run - no leak (but overall still failed)
    lsan.run([]()
    {
        int* ptr = new int(3);
        delete ptr;
    });
    QITI_REQUIRE(lsan.failed()); // Should remain failed
}

QITI_TEST_CASE("qiti::LeakSanitizer::complexAllocationPattern", LeakSanitizerComplexAllocationPattern)
{
    qiti::ScopedQitiTest test;
    
    qiti::LeakSanitizer lsan;
    lsan.run([]()
    {
        // Multiple allocations and deallocations
        int* ptr1 = new int(1);
        int* ptr2 = new int[50];
        int* ptr3 = new int(3);
        
        delete ptr1;
        delete[] ptr2;
        delete ptr3;
    });
    QITI_REQUIRE(lsan.passed());
}

QITI_TEST_CASE("qiti::LeakSanitizer::partialLeak", LeakSanitizerPartialLeak)
{
    qiti::ScopedQitiTest test;
    
    qiti::LeakSanitizer lsan;
    lsan.run([]()
    {
        int* ptr1 = new int(1);
        int* ptr2 = new int(2);
        int* ptr3 = new int(3);
        
        delete ptr1;
        delete ptr3;
        // ptr2 is leaked
        (void)ptr2;
    });
    QITI_REQUIRE(lsan.failed());
}

QITI_TEST_CASE("qiti::LeakSanitizer::moveConstructor", LeakSanitizerMoveConstructor)
{
    qiti::ScopedQitiTest test;
    
    qiti::LeakSanitizer lsan1;
    lsan1.run([]()
    {
        int* ptr = new int(42);
        // Leak intentionally
        (void)ptr;
    });
    QITI_REQUIRE(lsan1.failed());
    
    // Move construct
    qiti::LeakSanitizer lsan2 = std::move(lsan1);
    QITI_REQUIRE(lsan2.failed()); // Should preserve failed state
}

QITI_TEST_CASE("qiti::LeakSanitizer::moveAssignment", LeakSanitizerMoveAssignment)
{
    qiti::ScopedQitiTest test;
    
    qiti::LeakSanitizer lsan1;
    lsan1.run([]()
    {
        int* ptr = new int(42);
        // Leak intentionally
        (void)ptr;
    });
    QITI_REQUIRE(lsan1.failed());
    
    qiti::LeakSanitizer lsan2;
    QITI_REQUIRE(lsan2.passed()); // Initially passed
    
    // Move assign
    lsan2 = std::move(lsan1);
    QITI_REQUIRE(lsan2.failed()); // Should now be failed
}
#endif // __APPLE__

#pragma clang optimize on
