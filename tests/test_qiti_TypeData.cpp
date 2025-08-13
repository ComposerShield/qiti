// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"

// Qiti Private API - not included in qiti_include.hpp
#include "qiti_TypeData.hpp"

#include <string>
#include <vector>

//--------------------------------------------------------------------------

// Test classes for TypeData testing
class SimpleTestClass
{
public:
    int value = 42;
};

class LargeTestClass
{
public:
    char data[1024] = {};
    int id = 0;
};

struct SmallTestStruct
{
    char c = 'A';
};

//--------------------------------------------------------------------------

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated" // TODO: remove when TypeData is no longer deprecated

QITI_TEST_CASE("qiti::TypeData::getTypeData<T>()", TypeDataGetTypeData)
{
    qiti::ScopedQitiTest test;
    
    QITI_SECTION("Get TypeData for simple class")
    {
        auto* typeData = qiti::TypeData::getTypeData<SimpleTestClass>();
        QITI_REQUIRE(typeData != nullptr);
        
        // Verify type name contains the class name
        std::string typeName = typeData->getTypeName();
        QITI_REQUIRE(typeName.find("SimpleTestClass") != std::string::npos);
    }
    
    QITI_SECTION("Get TypeData for standard types")
    {
        auto* intData = qiti::TypeData::getTypeData<int>();
        QITI_REQUIRE(intData != nullptr);
        
        auto* stringData = qiti::TypeData::getTypeData<std::string>();
        QITI_REQUIRE(stringData != nullptr);
        
        // Different types should return different TypeData instances
        QITI_REQUIRE(intData != stringData);
    }
    
    QITI_SECTION("Multiple calls return same instance")
    {
        auto* typeData1 = qiti::TypeData::getTypeData<SimpleTestClass>();
        auto* typeData2 = qiti::TypeData::getTypeData<SimpleTestClass>();
        
        QITI_REQUIRE(typeData1 == typeData2);
    }
}

QITI_TEST_CASE("qiti::TypeData::getTypeSize()", TypeDataGetTypeSize)
{
    qiti::ScopedQitiTest test;
    
    QITI_SECTION("Verify type sizes")
    {
        auto* intData = qiti::TypeData::getTypeData<int>();
        QITI_REQUIRE(intData->getTypeSize() == sizeof(int));
        
        auto* simpleData = qiti::TypeData::getTypeData<SimpleTestClass>();
        QITI_REQUIRE(simpleData->getTypeSize() == sizeof(SimpleTestClass));
        
        auto* largeData = qiti::TypeData::getTypeData<LargeTestClass>();
        QITI_REQUIRE(largeData->getTypeSize() == sizeof(LargeTestClass));
        
        auto* smallData = qiti::TypeData::getTypeData<SmallTestStruct>();
        QITI_REQUIRE(smallData->getTypeSize() == sizeof(SmallTestStruct));
    }
}

QITI_TEST_CASE("qiti::TypeData::recordConstruction()", TypeDataRecordConstruction)
{
    qiti::ScopedQitiTest test;
    
    auto* typeData = qiti::TypeData::getTypeDataMutable<SimpleTestClass>();
    
    // Reset to ensure clean state
    typeData->reset();
    
    QITI_SECTION("Initial state")
    {
        QITI_REQUIRE(typeData->getNumConstructions() == 0);
        QITI_REQUIRE(typeData->getNumLiveInstances() == 0);
        QITI_REQUIRE(typeData->getPeakLiveInstances() == 0);
        QITI_REQUIRE(typeData->getTotalMemoryAllocated() == 0);
        QITI_REQUIRE(typeData->getCurrentMemoryUsed() == 0);
        QITI_REQUIRE(typeData->getPeakMemoryUsed() == 0);
    }
    
    QITI_SECTION("Single construction")
    {
        typeData->recordConstruction();
        
        QITI_REQUIRE(typeData->getNumConstructions() == 1);
        QITI_REQUIRE(typeData->getNumLiveInstances() == 1);
        QITI_REQUIRE(typeData->getPeakLiveInstances() == 1);
        QITI_REQUIRE(typeData->getTotalMemoryAllocated() == sizeof(SimpleTestClass));
        QITI_REQUIRE(typeData->getCurrentMemoryUsed() == sizeof(SimpleTestClass));
        QITI_REQUIRE(typeData->getPeakMemoryUsed() == sizeof(SimpleTestClass));
    }
    
    QITI_SECTION("Multiple constructions")
    {
        typeData->recordConstruction();
        typeData->recordConstruction();
        typeData->recordConstruction();
        
        QITI_REQUIRE(typeData->getNumConstructions() == 3);
        QITI_REQUIRE(typeData->getNumLiveInstances() == 3);
        QITI_REQUIRE(typeData->getPeakLiveInstances() == 3);
        QITI_REQUIRE(typeData->getTotalMemoryAllocated() == 3 * sizeof(SimpleTestClass));
        QITI_REQUIRE(typeData->getCurrentMemoryUsed() == 3 * sizeof(SimpleTestClass));
        QITI_REQUIRE(typeData->getPeakMemoryUsed() == 3 * sizeof(SimpleTestClass));
    }
}

QITI_TEST_CASE("qiti::TypeData::recordDestruction()", TypeDataRecordDestruction)
{
    qiti::ScopedQitiTest test;
    
    auto* typeData = qiti::TypeData::getTypeDataMutable<SimpleTestClass>();
    typeData->reset();
    
    QITI_SECTION("Construction followed by destruction")
    {
        typeData->recordConstruction();
        QITI_REQUIRE(typeData->getNumLiveInstances() == 1);
        QITI_REQUIRE(typeData->getCurrentMemoryUsed() == sizeof(SimpleTestClass));
        
        typeData->recordDestruction();
        QITI_REQUIRE(typeData->getNumConstructions() == 1);
        QITI_REQUIRE(typeData->getNumDestructions() == 1);
        QITI_REQUIRE(typeData->getNumLiveInstances() == 0);
        QITI_REQUIRE(typeData->getCurrentMemoryUsed() == 0);
        QITI_REQUIRE(typeData->getPeakLiveInstances() == 1); // Peak should remain
        QITI_REQUIRE(typeData->getPeakMemoryUsed() == sizeof(SimpleTestClass)); // Peak should remain
    }
    
    QITI_SECTION("Multiple constructions and partial destructions")
    {
        // Create 5 instances
        for (int i = 0; i < 5; ++i)
        {
            typeData->recordConstruction();
        }
        
        QITI_REQUIRE(typeData->getNumLiveInstances() == 5);
        QITI_REQUIRE(typeData->getPeakLiveInstances() == 5);
        
        // Destroy 2 instances
        typeData->recordDestruction();
        typeData->recordDestruction();
        
        QITI_REQUIRE(typeData->getNumConstructions() == 5);
        QITI_REQUIRE(typeData->getNumDestructions() == 2);
        QITI_REQUIRE(typeData->getNumLiveInstances() == 3);
        QITI_REQUIRE(typeData->getPeakLiveInstances() == 5); // Peak should remain
        QITI_REQUIRE(typeData->getCurrentMemoryUsed() == 3 * sizeof(SimpleTestClass));
    }
    
    QITI_SECTION("Destruction without construction")
    {
        // Should handle gracefully without going negative
        typeData->recordDestruction();
        
        QITI_REQUIRE(typeData->getNumConstructions() == 0);
        QITI_REQUIRE(typeData->getNumDestructions() == 1);
        QITI_REQUIRE(typeData->getNumLiveInstances() == 0);
        QITI_REQUIRE(typeData->getCurrentMemoryUsed() == 0);
    }
}

QITI_TEST_CASE("qiti::TypeData::peakTracking", TypeDataPeakTracking)
{
    qiti::ScopedQitiTest test;
    
    auto* typeData = qiti::TypeData::getTypeDataMutable<LargeTestClass>();
    typeData->reset();
    
    QITI_SECTION("Peak instance tracking")
    {
        // Create instances and track peak
        typeData->recordConstruction(); // 1 live
        QITI_REQUIRE(typeData->getPeakLiveInstances() == 1);
        
        typeData->recordConstruction(); // 2 live
        typeData->recordConstruction(); // 3 live
        QITI_REQUIRE(typeData->getPeakLiveInstances() == 3);
        
        typeData->recordDestruction(); // 2 live
        QITI_REQUIRE(typeData->getNumLiveInstances() == 2);
        QITI_REQUIRE(typeData->getPeakLiveInstances() == 3); // Peak remains
        
        typeData->recordConstruction(); // 3 live again
        QITI_REQUIRE(typeData->getPeakLiveInstances() == 3); // Peak unchanged
        
        typeData->recordConstruction(); // 4 live - new peak
        QITI_REQUIRE(typeData->getPeakLiveInstances() == 4);
    }
    
    QITI_SECTION("Peak memory tracking")
    {
        size_t instanceSize = sizeof(LargeTestClass);
        
        typeData->recordConstruction(); // 1 instance
        QITI_REQUIRE(typeData->getPeakMemoryUsed() == instanceSize);
        
        typeData->recordConstruction(); // 2 instances
        QITI_REQUIRE(typeData->getPeakMemoryUsed() == 2 * instanceSize);
        
        typeData->recordDestruction(); // 1 instance
        QITI_REQUIRE(typeData->getCurrentMemoryUsed() == instanceSize);
        QITI_REQUIRE(typeData->getPeakMemoryUsed() == 2 * instanceSize); // Peak remains
    }
}

QITI_TEST_CASE("qiti::TypeData::reset()", TypeDataReset)
{
    qiti::ScopedQitiTest test;
    
    auto* typeData = qiti::TypeData::getTypeDataMutable<SimpleTestClass>();
    
    QITI_SECTION("Reset after activity")
    {
        // Generate some activity
        typeData->recordConstruction();
        typeData->recordConstruction();
        typeData->recordDestruction();
        
        // Verify there's activity to reset
        QITI_REQUIRE(typeData->getNumConstructions() > 0);
        QITI_REQUIRE(typeData->getNumDestructions() > 0);
        QITI_REQUIRE(typeData->getPeakLiveInstances() > 0);
        
        // Reset and verify everything is zeroed
        typeData->reset();
        
        QITI_REQUIRE(typeData->getNumConstructions() == 0);
        QITI_REQUIRE(typeData->getNumDestructions() == 0);
        QITI_REQUIRE(typeData->getNumLiveInstances() == 0);
        QITI_REQUIRE(typeData->getPeakLiveInstances() == 0);
        QITI_REQUIRE(typeData->getTotalMemoryAllocated() == 0);
        QITI_REQUIRE(typeData->getCurrentMemoryUsed() == 0);
        QITI_REQUIRE(typeData->getPeakMemoryUsed() == 0);
        
        // Type information should remain
        QITI_REQUIRE(typeData->getTypeSize() == sizeof(SimpleTestClass));
        std::string typeName = typeData->getTypeName();
        QITI_REQUIRE(!typeName.empty());
    }
}

QITI_TEST_CASE("qiti::TypeData::multipleTypes", TypeDataMultipleTypes)
{
    qiti::ScopedQitiTest test;
    
    auto* intData = qiti::TypeData::getTypeDataMutable<int>();
    auto* stringData = qiti::TypeData::getTypeDataMutable<std::string>();
    auto* vectorData = qiti::TypeData::getTypeDataMutable<std::vector<int>>();
    
    // Reset all to ensure clean state
    intData->reset();
    stringData->reset();
    vectorData->reset();
    
    QITI_SECTION("Independent tracking")
    {
        // Activity on int type
        intData->recordConstruction();
        intData->recordConstruction();
        
        // Activity on string type
        stringData->recordConstruction();
        
        // Verify independence
        QITI_REQUIRE(intData->getNumConstructions() == 2);
        QITI_REQUIRE(stringData->getNumConstructions() == 1);
        QITI_REQUIRE(vectorData->getNumConstructions() == 0);
        
        QITI_REQUIRE(intData->getNumLiveInstances() == 2);
        QITI_REQUIRE(stringData->getNumLiveInstances() == 1);
        QITI_REQUIRE(vectorData->getNumLiveInstances() == 0);
    }
    
    QITI_SECTION("Different type sizes")
    {
        QITI_REQUIRE(intData->getTypeSize() == sizeof(int));
        QITI_REQUIRE(stringData->getTypeSize() == sizeof(std::string));
        QITI_REQUIRE(vectorData->getTypeSize() == sizeof(std::vector<int>));
        
        // Sizes should be different (at least int vs string/vector)
        QITI_REQUIRE(intData->getTypeSize() != stringData->getTypeSize());
    }
}

QITI_TEST_CASE("qiti::TypeData::getTypeDataMutable()", TypeDataGetTypeDataMutable)
{
    qiti::ScopedQitiTest test;
    
    QITI_SECTION("Mutable version should begin profiling")
    {
        // This should call Profile::beginProfilingType<T>() internally
        auto* typeData = qiti::TypeData::getTypeDataMutable<SimpleTestClass>();
        QITI_REQUIRE(typeData != nullptr);
        
        // Should be the same instance as regular getTypeData
        auto* typeData2 = qiti::TypeData::getTypeData<SimpleTestClass>();
        QITI_REQUIRE(typeData == typeData2);
    }
}

QITI_TEST_CASE("qiti::TypeData::memoryAccounting", TypeDataMemoryAccounting)
{
    qiti::ScopedQitiTest test;
    
    auto* typeData = qiti::TypeData::getTypeDataMutable<LargeTestClass>();
    typeData->reset();
    
    size_t instanceSize = sizeof(LargeTestClass);
    
    QITI_SECTION("Memory accounting accuracy")
    {
        // Create 3 instances
        typeData->recordConstruction();
        typeData->recordConstruction();
        typeData->recordConstruction();
        
        QITI_REQUIRE(typeData->getTotalMemoryAllocated() == 3 * instanceSize);
        QITI_REQUIRE(typeData->getCurrentMemoryUsed() == 3 * instanceSize);
        QITI_REQUIRE(typeData->getPeakMemoryUsed() == 3 * instanceSize);
        
        // Destroy 1 instance
        typeData->recordDestruction();
        
        QITI_REQUIRE(typeData->getTotalMemoryAllocated() == 3 * instanceSize); // Total never decreases
        QITI_REQUIRE(typeData->getCurrentMemoryUsed() == 2 * instanceSize);
        QITI_REQUIRE(typeData->getPeakMemoryUsed() == 3 * instanceSize); // Peak remains
        
        // Create 2 more instances (total 4 live)
        typeData->recordConstruction();
        typeData->recordConstruction();
        
        QITI_REQUIRE(typeData->getTotalMemoryAllocated() == 5 * instanceSize);
        QITI_REQUIRE(typeData->getCurrentMemoryUsed() == 4 * instanceSize);
        QITI_REQUIRE(typeData->getPeakMemoryUsed() == 4 * instanceSize); // New peak
    }
}

#pragma clang diagnostic pop
