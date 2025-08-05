
#include <cassert>
#include <cstring> // for std::strerror
#include <vector>
#include <string>

#if defined (DEBUG) || defined (_DEBUG) || ! (defined (NDEBUG) || defined (_NDEBUG))
    #define QITI_DEBUG_ASSERT(expression) assert(expression)
#else
    #define QITI_DEBUG_ASSERT(expression)
#endif // #if defined (DEBUG) || defined (_DEBUG)

//--------------------------------------------------------------------------
// Framework Detection and Abstraction Layer
//--------------------------------------------------------------------------

#ifdef QITI_USE_GTEST
    // GTest implementation with section emulation
    #include <gtest/gtest.h>
    
    namespace qiti::internal {
        class SectionManager {
            static inline thread_local std::vector<std::string> sections;
            static inline thread_local int current_section = -1;
            
        public:
            static void registerSection(const char* name) {
                sections.push_back(name);
            }
            
            static bool shouldRunSection(const char* name) {
                if (current_section == -1) {
                    // Discovery pass - register but don't run
                    registerSection(name);
                    return false;
                }
                return current_section < sections.size() && sections[current_section] == name;
            }
            
            static bool needsRestart() {
                current_section++;
                return current_section < sections.size();
            }
            
            static void reset() {
                sections.clear();
                current_section = -1;
            }
        };
    }
    
    // GTest test case macro with section support
    #define QITI_TEST_CASE(name) \
        void name##_impl(); \
        TEST(QitiTest, name) { \
            qiti::internal::SectionManager::reset(); \
            do { \
                name##_impl(); \
            } while (qiti::internal::SectionManager::needsRestart()); \
        } \
        void name##_impl()
    
    // GTest section macro  
    #define QITI_SECTION(name) \
        if (qiti::internal::SectionManager::shouldRunSection(name))
    
    // GTest assertion macros
    #define QITI_CHECK(expr) do { \
        QITI_DEBUG_ASSERT(expr); \
        EXPECT_TRUE(expr); \
    } while (false)
    
    #define QITI_REQUIRE(expr) do { \
        QITI_DEBUG_ASSERT(expr); \
        ASSERT_TRUE(expr); \
    } while (false)
    
    #define QITI_REQUIRE_FALSE(expr) do { \
        QITI_DEBUG_ASSERT(! (expr)); \
        ASSERT_FALSE(expr); \
    } while (false)

#else
    // Catch2 implementation (default)
    #include <catch2/catch_test_macros.hpp>
    
    // Catch2 test case macro
    #define QITI_TEST_CASE(name) TEST_CASE(name)
    
    // Catch2 section macro
    #define QITI_SECTION(name) SECTION(name)
    
    // Catch2 assertion macros with debug asserts
    #define QITI_CHECK(expr) do { \
        QITI_DEBUG_ASSERT(expr); \
        CHECK(expr); \
    } while (false)
    
    #define QITI_REQUIRE(expr) do { \
        QITI_DEBUG_ASSERT(expr); \
        REQUIRE(expr); \
    } while (false)
    
    #define QITI_REQUIRE_FALSE(expr) do { \
        QITI_DEBUG_ASSERT(! (expr)); \
        REQUIRE_FALSE(expr); \
    } while (false)

#endif // QITI_USE_GTEST
