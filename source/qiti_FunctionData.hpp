
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
        moveAssignment
    };
    
    /**
     Begins profiling for FuncPtr and returns a pointer to the corresponding FunctionData instance.
     
     FuncPtr can be a free function as well as member function.
     */
    template <auto FuncPtr>
    requires isFreeFunction<FuncPtr>
             || isMemberFunction<FuncPtr>
    [[nodiscard]] static const qiti::FunctionData* QITI_API getFunctionData() noexcept
    {
        return getFunctionDataMutable<FuncPtr>(); // wrap in const
    }
    
    /**
     Get the demangled name of the function.

     Returns a human-readable name for the function.
     */
    [[nodiscard]] const char* QITI_API getFunctionName() const noexcept;
    
    /**
     Get the total number of times this function was called.

     Returns the count of all recorded invocations that have occurred since we began profiling the function.
     */
    [[nodiscard]] uint64_t QITI_API getNumTimesCalled() const noexcept;
    
    /**
     Returns the average time spent inside this function, in nanoseconds.
     
     CPU time is the amount of time the thread actually spent executing
     on the CPU (user + kernel mode), and does ​not​ include time spent off-CPU.
     */
    [[nodiscard]] uint64_t QITI_API getAverageTimeSpentInFunctionCpu_ns() const noexcept;
    
    /**
     Returns the average time spent inside this function, in nanoseconds.
     
     Wall-clock time is the real-world elapsed time between entry and exit,
     and so includes any time the thread was preempted or blocked.
     */
    [[nodiscard]] uint64_t QITI_API getAverageTimeSpentInFunctionWallClock_ns() const noexcept;
    
    /**
     Check if the function was called on a specific thread.

     Returns true if any recorded invocation occurred on the thread identified by 'thread'.
     */
    [[nodiscard]] bool QITI_API wasCalledOnThread(std::thread::id thread) const noexcept;
    
    /**
     Retrieve the most recent function call data.

     Returns a FunctionCallData object representing the last recorded invocation of this function.
     */
    [[nodiscard]] FunctionCallData QITI_API getLastFunctionCall() const noexcept;
    
    /** */
    [[nodiscard]] static std::vector<const FunctionData*> QITI_API getAllProfiledFunctionData() noexcept;
    
    //--------------------------------------------------------------------------
    // Doxygen - Begin Internal Documentation
    /** \cond INTERNAL */
    //--------------------------------------------------------------------------
    
    struct Impl;
    /** */
    [[nodiscard]] Impl* QITI_API_INTERNAL getImpl() noexcept;
    /** */
    [[nodiscard]] const Impl* QITI_API_INTERNAL getImpl() const noexcept;
    
    /**
     Listener interface for function entry and exit events.

     Implement this interface to receive callbacks when a function begins or ends execution.
     */
    struct Listener
    {
        virtual ~Listener() = default;
        /** */
        virtual void QITI_API_INTERNAL onFunctionEnter(const FunctionData*) noexcept = 0;
        /** */
        virtual void QITI_API_INTERNAL onFunctionExit (const FunctionData*) noexcept = 0;
    };
    
    /** */
    void QITI_API_INTERNAL addListener(Listener* listener) noexcept;
    /** */
    void QITI_API_INTERNAL removeListener(Listener* listener) noexcept;
    
    /**
     Destroy this FunctionData instance.

     Cleans up any resources associated with tracking this function.
     */
    QITI_API_INTERNAL ~FunctionData() noexcept;
    
    /** Begins profiling for FuncPtr and returns a pointer to the corresponding mutable FunctionData instance. */
    template <auto FuncPtr>
    requires isFreeFunction<FuncPtr>
    [[nodiscard]] static qiti::FunctionData* QITI_API_INTERNAL getFunctionDataMutable() noexcept
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
    [[nodiscard]] FunctionData& QITI_API_INTERNAL operator=(FunctionData&& other) noexcept;
    
private:
    friend class Profile;
    friend class Utils;
    
    // Stack-based pimpl idiom
    static constexpr std::size_t ImplSize  = 504;
    static constexpr std::size_t ImplAlign = 8;
    alignas(ImplAlign) unsigned char implStorage[ImplSize];
    
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
    [[nodiscard]] static qiti::FunctionData* QITI_API_INTERNAL getFunctionDataMutable() noexcept
    {
        static constexpr auto functionAddress = Profile::getMemberFunctionMockAddress<FuncPtr>();
        static constexpr auto functionName    = Profile::getFunctionName<FuncPtr>();
        static constexpr auto functionType    = getFunctionType(functionName);
        beginProfilingFunction(functionAddress);
        return &Utils::getFunctionDataFromAddress(functionAddress,
                                                  functionName,
                                                  static_cast<int>(functionType));
    }
    
    /**
     Record that the function was called.

     Updates internal state to log a new invocation, including timestamp and thread information.
     */
    void QITI_API_INTERNAL functionCalled() noexcept;
    
    /** Copy Constructor (deleted) */
    FunctionData(const FunctionData&) = delete;
    /** Copy Assignment (deleted) */
    FunctionData& operator=(const FunctionData&) = delete;
    
    /** Prevent heap allocating this class (deleted) */
    void* operator new(std::size_t) = delete;
    /** Prevent heap allocating this class (deleted) */
    void* operator new[](std::size_t) = delete;
    
    /** */
    [[nodiscard]] static constexpr FunctionType QITI_API_INTERNAL getFunctionType(const char* functionName) noexcept
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
