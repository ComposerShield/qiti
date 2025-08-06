
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"

#include "qiti_LockData.hpp"

//--------------------------------------------------------------------------

/** Simple listener that records the last lock it saw acquired/released */
class TestListener : public qiti::LockData::Listener
{
public:
    const pthread_mutex_t* lastAcquire = nullptr;
    const pthread_mutex_t* lastRelease = nullptr;

    void onAcquire(const pthread_mutex_t* ld) noexcept override
    {
        lastAcquire = ld;
    }

    void onRelease(const pthread_mutex_t* ld) noexcept override
    {
        lastRelease = ld;
    }
};

QITI_TEST_CASE( "LockData delivers acquire and release to a single listener", LockDataSingleListener )
{
    qiti::ScopedQitiTest test;

    pthread_mutex_t* lock = nullptr; // contents of the pthread_mutex_t are ignored
    TestListener listener;

    qiti::LockData::addGlobalListener( &listener );

    qiti::LockData::notifyAcquire(lock);
    QITI_REQUIRE( listener.lastAcquire == lock );

    qiti::LockData::notifyRelease(lock);
    QITI_REQUIRE( listener.lastRelease == lock );

    qiti::LockData::removeGlobalListener( &listener );

    listener.lastAcquire = nullptr;
    listener.lastRelease = nullptr;

    qiti::LockData::notifyAcquire(nullptr);
    qiti::LockData::notifyRelease(nullptr);

    QITI_REQUIRE( listener.lastAcquire == nullptr );
    QITI_REQUIRE( listener.lastRelease == nullptr );
}

QITI_TEST_CASE( "LockData can handle multiple listeners", LockDataMultipleListeners )
{
    qiti::ScopedQitiTest test;

    pthread_mutex_t* lock = nullptr; // contents of the pthread_mutex_t are ignored
    TestListener a;
    TestListener b;

    qiti::LockData::addGlobalListener( &a );
    qiti::LockData::addGlobalListener( &b );

    qiti::LockData::notifyAcquire(lock);
    QITI_REQUIRE( a.lastAcquire == lock );
    QITI_REQUIRE( b.lastAcquire == lock );

    qiti::LockData::notifyRelease(lock);
    QITI_REQUIRE( a.lastRelease == lock );
    QITI_REQUIRE( b.lastRelease == lock );
    
    qiti::LockData::removeGlobalListener( &a );
    qiti::LockData::removeGlobalListener( &b );
}
