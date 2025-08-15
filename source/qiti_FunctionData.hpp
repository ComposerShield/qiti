
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_FunctionData.hpp
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

#include "qiti_FunctionCallData.hpp"
#include "qiti_Profile.hpp"
#include "qiti_Utils.hpp"
#include "qiti_ScopedNoHeapAllocations.hpp"

#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

//--------------------------------------------------------------------------

namespace qiti
{
//--------------------------------------------------------------------------
/**
 Abtracts a function and its history of use
 */
class FunctionData
{
public:
    enum class FunctionType
    {
        regular,
        constructor,
        destructor,
        copyConstructor,
        copyAssignment,
        moveConstructor,
        moveAssignment,
        
        unknown
    };
    
    /**
     Begins profiling for FuncPtr and returns a pointer to the corresponding FunctionData instance.
     
     FuncPtr can be a free function as well as member function.
     */
    template <auto FuncPtr>
    requires isFreeFunction<FuncPtr>
             || isMemberFunction<FuncPtr>
    [[nodiscard]] QITI_API_INLINE static const qiti::FunctionData* getFunctionData() noexcept
    {
        return getFunctionDataMutable<FuncPtr>(); // wrap in const
    }
    
    /**
     Get the demangled name of the function.

     Returns a human-readable name for the function.
     */
    [[nodiscard]] QITI_API const char* getFunctionName() const noexcept;
    
    /**
     Get the total number of times this function was called.

     Returns the count of all recorded invocations that have occurred since we began profiling the function.
     */
    [[nodiscard]] QITI_API uint64_t getNumTimesCalled() const noexcept;
    
    /**
     Returns the average time spent inside this function, in nanoseconds.
     
     CPU time is the amount of time the thread actually spent executing
     on the CPU (user + kernel mode), and does ​not​ include time spent off-CPU.
     
     Feature is not supported on Windows.
     */
#ifdef _WIN32
    [[deprecated("Feature is not supported on Windows")]]
#endif
    [[nodiscard]] QITI_API uint64_t getAverageTimeSpentInFunctionCpu_ns() const noexcept;
    
    /**
     Returns the average time spent inside this function, in nanoseconds.
     
     Wall-clock time is the real-world elapsed time between entry and exit,
     and so includes any time the thread was preempted or blocked.
     */
    [[nodiscard]] QITI_API uint64_t getAverageTimeSpentInFunctionWallClock_ns() const noexcept;
    
    /**
     Returns the minimum CPU time spent in any single call to this function, in nanoseconds.
     
     CPU time is the amount of time the thread actually spent executing
     on the CPU (user + kernel mode), and does ​not​ include time spent off-CPU.
     Returns 0 if the function has never been called.
     
     Feature is not supported on Windows.
     */
#ifdef _WIN32
    [[deprecated("Feature is not supported on Windows")]]
#endif
    [[nodiscard]] QITI_API uint64_t getMinTimeSpentInFunctionCpu_ns() const noexcept;
    
    /**
     Returns the maximum CPU time spent in any single call to this function, in nanoseconds.
     
     CPU time is the amount of time the thread actually spent executing
     on the CPU (user + kernel mode), and does ​not​ include time spent off-CPU.
     Returns 0 if the function has never been called.
     */
    [[nodiscard]] QITI_API uint64_t getMaxTimeSpentInFunctionCpu_ns() const noexcept;
    
    /**
     Returns the minimum wall-clock time spent in any single call to this function, in nanoseconds.
     
     Wall-clock time is the real-world elapsed time between entry and exit,
     and so includes any time the thread was preempted or blocked.
     Returns 0 if the function has never been called.
     */
    [[nodiscard]] QITI_API uint64_t getMinTimeSpentInFunctionWallClock_ns() const noexcept;
    
    /**
     Returns the maximum wall-clock time spent in any single call to this function, in nanoseconds.
     
     Wall-clock time is the real-world elapsed time between entry and exit,
     and so includes any time the thread was preempted or blocked.
     Returns 0 if the function has never been called.
     */
    [[nodiscard]] QITI_API uint64_t getMaxTimeSpentInFunctionWallClock_ns() const noexcept;
    
    /**
     Check if the function was called on a specific thread.

     Returns true if any recorded invocation occurred on the thread identified by 'thread'.
     */
    [[nodiscard]] QITI_API bool wasCalledOnThread(std::thread::id thread) const noexcept;
    
    /**
     Retrieve the most recent function call data.

     Returns a FunctionCallData object representing the last recorded invocation of this function.
     */
    [[nodiscard]] QITI_API FunctionCallData getLastFunctionCall() const noexcept;
    
    /**
     Get all profiled function data.

     Returns a vector of pointers to the FunctionData instances for each function currently being profiled.
     */
    [[nodiscard]] QITI_API static std::vector<const FunctionData*> getAllProfiledFunctionData() noexcept;
    
    /**
     Get all functions that have called this function.
     
     @returns A vector of pointers to FunctionData instances representing all functions
     that have called this function at least once during profiling. The vector may be 
     empty if this function was only called from outside the profiled call stack.
     
     Note: This only works reliably when you have called ScopedQitiTest::enableProfilingOnAllFunctions(true).
     */
    [[nodiscard]] QITI_API std::vector<const FunctionData*> getCallers() const noexcept;
    
    /**
     @returns The total number of exceptions thrown by this function.
     
     Counts the number of times this specific function executed a throw statement.
     This does not include exceptions thrown by other functions that this function calls.
     @returns 0 if no exceptions have been thrown by this function.
     */
    [[nodiscard]] QITI_API uint64_t getNumExceptionsThrown() const noexcept;
    
    /**
     @returns True if this function is any type of constructor.
     
     Example: `MyClass()`, `MyClass(const MyClass&)`, `MyClass(MyClass&&)`
     */
    [[nodiscard]] QITI_API bool isConstructor() const noexcept;

    /**
     @returns True if this function is a regular (non-copy, non-move) constructor.
     
     Example: `MyClass()`, `MyClass(int x, float y)`
     */
    [[nodiscard]] QITI_API bool isRegularConstructor() const noexcept;

    /**
     @returns True if this function is a copy constructor.
     
     Example: `MyClass(const MyClass& other)`
     */
    [[nodiscard]] QITI_API bool isCopyConstructor() const noexcept;

    /**
     @returns True if this function is a move constructor.
     
     Example: `MyClass(MyClass&& other)`
     */
    [[nodiscard]] QITI_API bool isMoveConstructor() const noexcept;

    /**
     @returns True if this function is any type of assignment operator.
     
     Example: `operator=(const MyClass&)`, `operator=(MyClass&&)`
     */
    [[nodiscard]] QITI_API bool isAssignment() const noexcept;

    /**
     @returns True if this function is a copy assignment operator.
     
     Example: `MyClass& operator=(const MyClass& other)`
     */
    [[nodiscard]] QITI_API bool isCopyAssignment() const noexcept;

    /**
     @returns True if this function is a move assignment operator.
     
     Example: `MyClass& operator=(MyClass&& other)`
     */
    [[nodiscard]] QITI_API bool isMoveAssignment() const noexcept;

    /**
     @returns True if this function is a destructor.
     
     Example: `~MyClass()`
     */
    [[nodiscard]] QITI_API bool isDestructor() const noexcept;
    
    //--------------------------------------------------------------------------
    // Doxygen - Begin Internal Documentation
    /** \cond INTERNAL */
    //--------------------------------------------------------------------------
    
    struct Impl;
    /** */
    [[nodiscard]] QITI_API_INTERNAL Impl* getImpl() noexcept;
    /** */
    [[nodiscard]] QITI_API_INTERNAL const Impl* getImpl() const noexcept;
    
    /**
     Listener interface for function entry and exit events.

     Implement this interface to receive callbacks when a function begins or ends execution.
     */
    struct Listener
    {
        virtual ~Listener() = default;
        /** */
        QITI_API_INTERNAL virtual void onFunctionEnter(const FunctionData*) noexcept = 0;
        /** */
        QITI_API_INTERNAL virtual void onFunctionExit (const FunctionData*) noexcept = 0;
    };
    
    /** */
    QITI_API_INTERNAL void addListener(Listener* listener) noexcept;
    /** */
    QITI_API_INTERNAL void removeListener(Listener* listener) noexcept;
    
    /**
     Destroy this FunctionData instance.

     Cleans up any resources associated with tracking this function.
     */
    QITI_API_INTERNAL ~FunctionData() noexcept;
    
    /** Begins profiling for FuncPtr and returns a pointer to the corresponding mutable FunctionData instance. */
    template <auto FuncPtr>
    requires isFreeFunction<FuncPtr>
    [[nodiscard]] QITI_API_INTERNAL static qiti::FunctionData* getFunctionDataMutable() noexcept
    {
        static constexpr auto functionAddress = Profile::getFunctionAddress<FuncPtr>();
        static constexpr auto functionName    = Profile::getFunctionName<FuncPtr>();
        static constexpr auto functionType    = getFunctionType(functionName);
        qiti::Profile::beginProfilingFunction<FuncPtr>();
        return &Utils::getFunctionDataFromAddress(functionAddress,
                                                  functionName,
                                                  static_cast<int>(functionType));
    }
    
    /** Move Constructor */
    QITI_API_INTERNAL FunctionData(FunctionData&& other) noexcept;
    /** Move Assignment */
    [[nodiscard]] QITI_API_INTERNAL FunctionData& operator=(FunctionData&& other) noexcept;
    
private:
    friend class Profile;
    friend class Utils;
    
    std::unique_ptr<Impl> pImpl;
    
    /**
     Construct FunctionData for a raw function address.

     Initializes internal tracking structures using the provided function address.
     */
    QITI_API_INTERNAL FunctionData(const void*  functionAddress,
                                   const char*  functionName,
                                   FunctionType functionType) noexcept;
    
    /**
     Begins profiling for FuncPtr and returns a pointer to the corresponding mutable FunctionData instance.
     
     Member function overload.
     */
    template <auto FuncPtr>
    requires isMemberFunction<FuncPtr>
    [[nodiscard]] QITI_API_INTERNAL static qiti::FunctionData* getFunctionDataMutable() noexcept
    {
        static constexpr auto functionAddress = Profile::getMemberFunctionMockAddress<FuncPtr>();
        static constexpr auto functionName    = Profile::getFunctionName<FuncPtr>();
        static constexpr auto functionType    = getFunctionType(functionName);
        qiti::Profile::beginProfilingFunction(functionAddress);
        return &Utils::getFunctionDataFromAddress(functionAddress,
                                                  functionName,
                                                  static_cast<int>(functionType));
    }
    
    /**
     Record that the function was called.

     Updates internal state to log a new invocation, including timestamp and thread information.
     */
    QITI_API_INTERNAL void functionCalled() noexcept;
    
    /** Copy Constructor (deleted) */
    FunctionData(const FunctionData&) = delete;
    /** Copy Assignment (deleted) */
    FunctionData& operator=(const FunctionData&) = delete;
    
    
    /** */
    [[nodiscard]] QITI_API_INTERNAL static constexpr FunctionType getFunctionType(const char* functionName) noexcept
    {
        std::string_view func_sv{functionName};

        // Find the opening '(' of the parameter list
        auto paren = func_sv.find('(');
        if (paren != std::string_view::npos)
        {
            // Split into:
            //    before  = "Qualifier::LastPart"    (everything before '(')
            //    params  = "(...)"                   (including parentheses)
            std::string_view before = func_sv.substr(0, paren);
            std::string_view params = func_sv.substr(paren); // e.g. "(const MyClass&)" or "(MyClass&&)"

            // Find the last "::" in the prefix to separate qualifier vs. last_part
            auto colcol = before.rfind("::");
            if (colcol != std::string_view::npos)
            {
                std::string_view qualifier = before.substr(0, colcol);
                std::string_view last_part  = before.substr(colcol + 2);

                // ---- Constructor family (name == qualifier) ----
                if (last_part == qualifier)
                {
                    // Look for a parameter that is the same type as 'qualifier'
                    auto pos = params.find(qualifier);
                    if (pos != std::string_view::npos)
                    {
                        // Move‐constructor: parameter contains "Qualifier&&"
                        if (params.find("&&", pos + qualifier.size()) != std::string_view::npos)
                        {
                            return FunctionType::moveConstructor;
                        }
                        // Copy‐constructor: parameter contains "Qualifier&" (but not "&&")
                        else if (params.find("&", pos + qualifier.size()) != std::string_view::npos)
                        {
                            return FunctionType::copyConstructor;
                        }
                    }
                    // Else: it's “someClass::someClass(...)” but not copy/move (e.g. default or other ctor)
                    return FunctionType::constructor;
                }

                // ---- Destructor (last_part starts with '~' and matches qualifier) ----
                else if (!last_part.empty() && last_part[0] == '~'
                         && qualifier == last_part.substr(1))
                {
                    return FunctionType::destructor;
                }

                // ---- Assignment‐operator family ----
                else if (last_part == "operator=")
                {
                    // Again, look for qualifier in the parameter list
                    auto pos = params.find(qualifier);
                    if (pos != std::string_view::npos)
                    {
                        // Move‐assignment: "(someClass&&)"
                        if (params.find("&&", pos + qualifier.size()) != std::string_view::npos)
                        {
                            return FunctionType::moveAssignment;
                        }
                        // Copy‐assignment: "(const someClass&)" or "(someClass&)"
                        else if (params.find("&", pos + qualifier.size()) != std::string_view::npos)
                        {
                            return FunctionType::copyAssignment;
                        }
                    }
                    // If there is an operator= but the parameter didn’t clearly match qualifier& or qualifier&&,
                    // fall through to regular.
                    return FunctionType::regular;
                }
            }
        }

        // ---- If none of the above matched, it's just a normal function ----
        return FunctionType::regular;
    }
    
    //--------------------------------------------------------------------------
    /** \endcond */
    // Doxygen - End Internal Documentation
    //--------------------------------------------------------------------------
};
} // namespace qiti
