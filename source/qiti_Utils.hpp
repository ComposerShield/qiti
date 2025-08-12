
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_Utils.hpp
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

#include "qiti_Profile.hpp"

#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
// Windows equivalent of dladdr
struct Dl_info 
{
    const char* dli_fname;  // Pathname of shared library
    void* dli_fbase;        // Base address of shared library
    const char* dli_sname;  // Name of nearest symbol
    void* dli_saddr;        // Address of nearest symbol
};
int dladdr(const void* addr, Dl_info* info);
#else
#include <dlfcn.h>
#endif

#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

//--------------------------------------------------------------------------
// Doxygen - Begin Internal Documentation
/** \cond INTERNAL */
//--------------------------------------------------------------------------
namespace qiti
{
//--------------------------------------------------------------------------
class FunctionData;
//--------------------------------------------------------------------------
/** Utility class for function‐metadata utilities and profiling support. */
class Utils
{
public:
    /** Reset all profiling and instrumentation data (including function data mapping) */
    QITI_API_INTERNAL static void resetAll() noexcept;
    
    template <auto FuncPtr>
    requires isFreeFunction<FuncPtr>
    QITI_API [[nodiscard]] static const qiti::FunctionData* getFunctionData() noexcept
    {
        static constexpr auto functionAddress = Profile::getFunctionAddress<FuncPtr>();
        static constexpr auto functionName    = Profile::getFunctionName<FuncPtr>();
        return &getFunctionDataFromAddress(functionAddress, functionName);
    }
    
    /** */
    QITI_API [[nodiscard]] static std::vector<const qiti::FunctionData*> getAllFunctionData() noexcept;
    
    /** demangle a GCC/Clang‐mangled name into a std::string */
    QITI_API_INTERNAL static void demangle(const char* mangled_name,
                                           char* demangled_name,
                                           uint64_t demangled_size) noexcept;
    
private:
    friend class FunctionData;
    friend class Profile;
    
    Utils() = delete;
    ~Utils() = delete;
    
    /** Likely never used. */
    QITI_API_INTERNAL static void* getAddressForMangledFunctionName(const char* mangledName) noexcept;
    
    /** */
    QITI_API [[nodiscard]] static qiti::FunctionData& getFunctionDataFromAddress(const void* functionAddress,
                                                                                 const char* functionName = nullptr,
                                                                                 int functionType = -1) noexcept;
    
    /** */
    [[nodiscard]] static const qiti::FunctionData* QITI_API getFunctionData(const char* demangledFunctionName) noexcept;
    
}; // class Utils
}  // namespace qiti

//--------------------------------------------------------------------------
/** \endcond */
// Doxygen - End Internal Documentation
//--------------------------------------------------------------------------
