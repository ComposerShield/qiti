
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_utils.hpp
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

#include <dlfcn.h>

#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>

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
    /** */
    static void QITI_API_INTERNAL resetAll() noexcept;
    
    /**
     Copies up to maxFunctions names (each truncated to maxNameLen–1 chars + '\0')
     into a single flat buffer of size maxFunctions * maxNameLen.
     Returns the actual number of names written.
     
     Call example:
     constexpr size_t MAX_FUNCS = 128;
     constexpr size_t MAX_NAME_LEN = 64;
     char buffer[MAX_FUNCS * MAX_NAME_LEN];
     getAllKnownFunctions(buffer, MAX_FUNCS, MAX_NAME_LEN);
     */
    static uint64_t QITI_API getAllKnownFunctions(char* buffer,
                                                  uint64_t maxFunctions,
                                                  uint64_t maxNameLen) noexcept;
    
    /** */
    static void* QITI_API getAddressForMangledFunctionName(const char* mangledName) noexcept;
    
    template <auto FuncPtr>
    requires std::is_function_v<std::remove_pointer_t<decltype(FuncPtr)>>
    [[nodiscard]] static const qiti::FunctionData* QITI_API getFunctionData() noexcept
    {
        static constexpr auto functionAddress = Profile::getFunctionAddress<FuncPtr>();
        static constexpr auto functionName    = Profile::getFunctionName<FuncPtr>();
        return &getFunctionDataFromAddress(functionAddress, functionName);
    }
    
private:
    //--------------------------------------------------------------------------
    // Doxygen - Begin Internal Documentation
    /** \cond INTERNAL */
    //--------------------------------------------------------------------------
    
    friend class FunctionData;
    friend class Profile;
    
    Utils() = delete;
    ~Utils() = delete;
    
    /** demangle a GCC/Clang‐mangled name into a std::string */
    static void QITI_API demangle(const char* mangled_name,
                                  char* demangled_name,
                                  uint64_t demangled_size) noexcept;
    
    /** */
    [[nodiscard]] static qiti::FunctionData& QITI_API getFunctionDataFromAddress(const void* functionAddress,
                                                                                 const char* functionName = nullptr,
                                                                                 int functionType = 0) noexcept;
    
    /** */
    [[nodiscard]] static const qiti::FunctionData* QITI_API getFunctionData(const char* demangledFunctionName) noexcept;
    
    //--------------------------------------------------------------------------
    /** \endcond */
    // Doxygen - End Internal Documentation
    //--------------------------------------------------------------------------
    
}; // class Utils
}  // namespace qiti
