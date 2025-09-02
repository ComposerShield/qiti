
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_profile.hpp
 *
 * @author   Adam Shield
 * @date     2025-05-16
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#pragma once

#include "qiti_API.hpp"

#include <array>
#include <cstdint>
#include <stack>
#include <string>
#include <type_traits>
#include <typeindex>
#include <typeinfo>

namespace qiti
{
//--------------------------------------------------------------------------
// Doxygen - Begin Internal Documentation
/** \cond INTERNAL */
//--------------------------------------------------------------------------

class FunctionData;
// Forward declaration for external access
extern thread_local std::stack<qiti::FunctionData*> g_callStack;

namespace FunctionNameHelpers
{
    /** A constexpr helper that slices out “FuncPtr = …” from __PRETTY_FUNCTION__ */
    template <auto FuncPtr>
    [[nodiscard]] QITI_API_INLINE consteval auto makeFunctionNameArray() noexcept
    {
        constexpr const char* fullFuncName = __PRETTY_FUNCTION__;
        constexpr std::string_view pretty = fullFuncName;
        
        constexpr std::string_view marker = "FuncPtr = ";
        constexpr std::size_t start = pretty.find(marker) + marker.size();
        constexpr std::size_t end   = pretty.rfind(']');
        static_assert(start != std::string_view::npos, "Could not find “FuncPtr = ”");
        static_assert(end   != std::string_view::npos, "Could not find trailing ‘]’");
        
        constexpr std::string_view raw = pretty.substr(start, end - start);
        constexpr std::string_view funcPtrString =
        (! raw.empty() && raw.front() == '&')
        ? raw.substr(1)
        : raw;
        
        std::array<char, funcPtrString.size() + 1> tmp = {};
        for (std::size_t i = 0; i < funcPtrString.size(); ++i)
        {
            tmp[i] = funcPtrString[i];
        }
        tmp[funcPtrString.size()] = '\0';
        return tmp;
    }
    
    // Each FuncPtr gets exactly one array-of-chars at namespace scope:
    template <auto FuncPtr>
    inline static constexpr auto functionNameArray = makeFunctionNameArray<FuncPtr>();
    
    // Constexpr pointer into that array:
    template <auto FuncPtr>
    inline static constexpr const char* functionNameCStr = functionNameArray<FuncPtr>.data();
} // namespace FunctionNameHelpers

//--------------------------------------------------------------------------

namespace TypeNameHelpers
{
    /** A constexpr helper that extracts type name from __PRETTY_FUNCTION__ */
    template <typename T>
    [[nodiscard]] QITI_API_INLINE consteval auto makeTypeNameArray() noexcept
    {
        constexpr const char* fullFuncName = __PRETTY_FUNCTION__;
        constexpr std::string_view pretty = fullFuncName;
        
        constexpr std::string_view marker = "T = ";
        constexpr std::size_t start = pretty.find(marker) + marker.size();
        constexpr std::size_t end   = pretty.rfind(']');
        static_assert(start != std::string_view::npos, "Could not find T = marker");
        static_assert(end   != std::string_view::npos, "Could not find trailing ']'");
        
        constexpr std::string_view typeString = pretty.substr(start, end - start);
        
        std::array<char, typeString.size() + 1> tmp = {};
        for (std::size_t i = 0; i < typeString.size(); ++i)
        {
            tmp[i] = typeString[i];
        }
        tmp[typeString.size()] = '\0';
        return tmp;
    }
    
    // Each type T gets exactly one array-of-chars at namespace scope:
    template <typename T>
    inline static constexpr auto typeNameArray = makeTypeNameArray<T>();
    
    // Constexpr pointer into that array:
    template <typename T>
    inline static constexpr const char* typeNameCStr = typeNameArray<T>.data();
} // namespace TypeNameHelpers

//--------------------------------------------------------------------------
/** \endcond */
// Doxygen - End Internal Documentation
//--------------------------------------------------------------------------

/**
 Utility class for runtime profiling of functions and types.
 
 The Profile class provides a convenient interface for instrumenting and querying profiling
 data at runtime.
 */
class Profile
{
public:
    /** RAII Object for disabling Profiling for the duration of a qiti function */
    struct ScopedDisableProfiling final
    {
        QITI_API ScopedDisableProfiling() noexcept;
        QITI_API ~ScopedDisableProfiling() noexcept;
        
    private:
        const bool wasProfilingEnabled;
    };
    
    /**
     Resets all profiling data and stops profiling all functions.
     
     Clears all accumulated profiling data and disables profiling for all functions.
     Called automatically when ScopedQitiTest goes out of scope.
     */
    QITI_API static void resetProfiling() noexcept;
    
    /**
     Begins profiling for a free function.
     
     This function is automatically called by FunctionData::getFunctionData().
     It is recommended to use getFunctionData() instead of calling this directly.
     */
    template<auto FuncPtr>
    requires isFreeFunction<FuncPtr>
    QITI_API_INLINE static void inline beginProfilingFunction() noexcept
    {
        static constexpr auto functionAddress = getFunctionAddress<FuncPtr>();
        static constexpr auto functionName    = getFunctionName<FuncPtr>();
        beginProfilingFunction(functionAddress, functionName);
    }
    
    /**
     Begins profiling for a member function of a class/struct.
     
     This function is automatically called by FunctionData::getFunctionData().
     It is recommended to use getFunctionData() instead of calling this directly.
     */
    template<auto FuncPtr>
    requires isMemberFunction<FuncPtr>
    QITI_API_INLINE static void inline beginProfilingFunction() noexcept
    {
        static constexpr auto functionAddress = getMemberFunctionMockAddress<FuncPtr>();
        static constexpr auto functionName    = getFunctionName<FuncPtr>();
        beginProfilingFunction(functionAddress, functionName);
    }
    
    /**
     Ends profiling for a free function.
     
     Stops profiling the specified free function. Note that getFunctionData()
     does not automatically call this - functions remain profiled until
     resetProfiling() is called or the ScopedQitiTest goes out of scope.
     */
    template <auto FuncPtr>
    requires isFreeFunction<FuncPtr>
    QITI_API_INLINE static void inline endProfilingFunction() noexcept
    {
        endProfilingFunction(reinterpret_cast<const void*>(FuncPtr));
    }
    
    /**
     Ends profiling for a member function of a class/struct.
     
     Stops profiling the specified member function. Note that getFunctionData()
     does not automatically call this - functions remain profiled until
     resetProfiling() is called or the ScopedQitiTest goes out of scope.
     */
    template <auto FuncPtr>
    requires isMemberFunction<FuncPtr>
    QITI_API_INLINE static void inline endProfilingFunction() noexcept
    {
        endProfilingFunction(getMemberFunctionMockAddress<FuncPtr>());
    }
    
    /**
     Enables automatic profiling for all functions.
     
     Once called, any time a function is called in your code, it will automatically
     become profiled by Qiti without needing to explicitly call getFunctionData().
     This allows for comprehensive profiling of the entire call stack.
     */
    QITI_API static void beginProfilingAllFunctions() noexcept;
    
    /**
     Disables automatic profiling for all functions.
     
     Stops the automatic profiling started by beginProfilingAllFunctions().
     Functions that were already being profiled explicitly will continue to be profiled.
     */
    QITI_API static void endProfilingAllFunctions() noexcept;
    
    /** @returns true if we are currently profling function. */
    template<auto FuncPtr>
    requires isFreeFunction<FuncPtr>
    [[nodiscard]] QITI_API_INLINE static inline bool isProfilingFunction() noexcept
    {
        return isProfilingFunction(reinterpret_cast<const void*>(FuncPtr));
    }
    
    /**
     @returns true if we are currently profling function.
     Member function overload.
     */
    template<auto FuncPtr>
    requires isMemberFunction<FuncPtr>
    [[nodiscard]] QITI_API_INLINE static inline bool isProfilingFunction() noexcept
    {
        return isProfilingFunction(getMemberFunctionMockAddress<FuncPtr>());
    }
    
    /**
     @internal
     TODO: Begins profiling for a specific type.
     
     This feature is not yet implemented.
     */
    template<typename Type>
    QITI_API_INLINE static inline void beginProfilingType() noexcept { beginProfilingType( typeid(Type) ); }
    
    /**
     @internal
     TODO: Ends profiling for a specific type.
     
     This feature is not yet implemented.
     */
    template <typename Type>
    QITI_API_INLINE static inline void endProfilingType() noexcept { endProfilingType( typeid(Type) ); }
    
    /**
     Gets the total number of heap allocations made on the current thread.
     
     @returns The total count of heap allocations (malloc, new, etc.) made
              on the current thread since profiling began.
     */
    [[nodiscard]] QITI_API static uint64_t getNumHeapAllocationsOnCurrentThread() noexcept;
    
    /**
     Gets the total amount of memory allocated on the heap for the current thread.
     
     @returns The total number of bytes allocated on the heap for the current
              thread since profiling began.
     */
    [[nodiscard]] QITI_API static uint64_t getAmountHeapAllocatedOnCurrentThread() noexcept;
    
    /**
     Gets the compile-time demangled name of a function.
     
     @returns A human-readable string containing the function name, extracted
              at compile-time from compiler intrinsics.
     */
    template<auto FuncPtr>
    requires isFreeFunction<FuncPtr>
    || isMemberFunction<FuncPtr>
    [[nodiscard]] QITI_API_INLINE static consteval const char* getFunctionName() noexcept
    {
        return FunctionNameHelpers::functionNameCStr<FuncPtr>;
    }
    
    /**
     Gets the compile-time demangled name of a type.
     
     @returns A human-readable string containing the type name, extracted
              at compile-time from compiler intrinsics.
     */
    template<typename T>
    [[nodiscard]] QITI_API_INLINE static consteval const char* getTypeName() noexcept
    {
        return TypeNameHelpers::typeNameCStr<T>;
    }
    
private:
    //--------------------------------------------------------------------------
    // Doxygen - Begin Internal Documentation
    /** \cond INTERNAL */
    //--------------------------------------------------------------------------
    
    friend class FunctionData;
    friend class FunctionDataUtils;
    friend class Instrument;
    friend class InstrumentHooks;

    Profile() = delete;
    ~Profile() = delete;
    
    /**
     Updates profiling data when a function is entered.

     This internal hook is called when function entry is detected through
     instrumentation. It updates call counts, timing information, and manages
     the call stack for nested function tracking.

     @param this_fn Pointer to the function being entered
     @note This function is designed for internal use by the Qiti profiling system.
     */
    QITI_API_INTERNAL static void updateFunctionDataOnEnter(const void* this_fn) noexcept;
    
    /**
     Updates profiling data when a function is exited.

     This internal hook is called when function exit is detected through
     instrumentation. It finalizes timing measurements, updates call counts,
     and manages the call stack by removing the current function from tracking.

     @param this_fn Pointer to the function being exited
     @note This function is designed for internal use by the Qiti profiling system.
     */
    QITI_API_INTERNAL static void updateFunctionDataOnExit(const void* this_fn) noexcept;

    /**
     Compile-time storage for free function addresses used in profiling.

     This struct provides a compile-time mechanism to store and retrieve the
     address of free functions for profiling purposes. Each function pointer
     gets its own unique instantiation, ensuring that function addresses can
     be resolved at compile time for efficient runtime profiling.

     @note This struct is designed for internal use by the Qiti profiling system.
     */
    template <auto FuncPtr>
    requires isFreeFunction<FuncPtr>
    struct FunctionAddressHolder
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
        static constexpr const void* value = (const void*)FuncPtr; // NOLINT
#pragma clang diagnostic pop
    };
    
    /**
     Mock address storage for member functions used in profiling.

     Since C++ does not portably allow direct function pointers to member
     functions, this struct provides a workaround by creating a unique static
     variable for each member function. The address of this variable serves
     as a unique identifier for profiling member functions.

     @note This struct is designed for internal use by the Qiti profiling system.
     */
    template <auto FuncPtr>
    requires isMemberFunction<FuncPtr>
    struct MemberFunctionMockAddressHolder
    {
        static constexpr void* value = nullptr;
    };
    
    /**
     Returns a mock "address" we can use for member functions.
     C++ does not (portably) allow function pointers to member functions.
     Addresses of static variables are guaranteed to be unique so we can
     create one for each member function we wish to profile.
     */
    template <auto FuncPtr>
    requires isFreeFunction<FuncPtr>
    [[nodiscard]] QITI_API_INLINE static consteval const void* getFunctionAddress() noexcept
    {
        return FunctionAddressHolder<FuncPtr>::value;
    }
    
    /**
     Returns a mock "address" we can use for member functions.
     C++ does not (portably) allow function pointers to member functions.
     Addresses of static variables are guaranteed to be unique so we can
     create one for each member function we wish to profile.
     */
    template <auto FuncPtr>
    requires isMemberFunction<FuncPtr>
    [[nodiscard]] QITI_API_INLINE static consteval const void* getMemberFunctionMockAddress() noexcept
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
        // c-style cast required because reinterpret_cast cannot be performed at compile-time
        return (const void*)(&MemberFunctionMockAddressHolder<FuncPtr>::value); // NOLINT
#pragma clang diagnostic pop
    }
    
    /** */
    QITI_API static void beginProfilingFunction(const void* functionAddress, const char* functionName = nullptr) noexcept;
    
    /** */
    QITI_API static void endProfilingFunction(const void* functionAddress) noexcept;
    
    /** */
    QITI_API static void beginProfilingType(std::type_index functionAddress) noexcept;
    
    /** */
    QITI_API static void endProfilingType(std::type_index functionAddress) noexcept;
    
    /**
     @returns true if we are currently profling function.
     */
    [[nodiscard]] QITI_API static bool isProfilingFunction(const void* funcAddress) noexcept;
    
    //--------------------------------------------------------------------------
    /** \endcond */
    // Doxygen - End Internal Documentation
    //--------------------------------------------------------------------------
}; // class Profile
}  // namespace qiti
