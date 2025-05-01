
#pragma once

#ifndef QITI_API_INTERNAL
  #define QITI_API_INTERNAL __attribute__((no_instrument_function))
#endif

#if defined _WIN32 || defined __CYGWIN__
  #ifdef QITI_DYLIB
    #define QITI_API QITI_API_INTERNAL __declspec(dllexport)
  #else
    #define QITI_API QITI_API_INTERNAL __declspec(dllimport)
  #endif
#else
  #ifdef QITI_DYLIB
    #define QITI_API QITI_API_INTERNAL __attribute__((visibility("default")))
  #else
    #define QITI_API QITI_API_INTERNAL
  #endif
#endif

#include <dlfcn.h>
#include <string>

//--------------------------------------------------------------------------

namespace qiti
{
//--------------------------------------------------------------------------

using uint = unsigned long long;

class FunctionData;

//--------------------------------------------------------------------------
template <auto FuncPtr>
constexpr std::string_view getFunctionName()
{
#if defined(__clang__) || defined(__GNUC__)
    constexpr std::string_view full   = __PRETTY_FUNCTION__;
    constexpr std::string_view prefix = "FuncPtr = ";
    auto pos = full.find(prefix);
    if (pos == std::string_view::npos) return {};

    auto rest = full.substr(pos + prefix.size());
    auto end  = rest.find(']');
    auto label = rest.substr(0, end);

    // strip leading '&' if present
    if (!label.empty() && label.front() == '&')
        label.remove_prefix(1);

    return label;
#elif defined(_MSC_VER)
    // similar parsing on __FUNCSIG__
    // ...
#endif

    return {};
}


/** demangle a GCC/Clang‐mangled name into a std::string */
void QITI_API demangle(const char* mangled_name,
                       char* demangled_name,
                       uint demangled_size);


/** */
void QITI_API shutdown();

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
uint QITI_API getAllKnownFunctions(char* buffer,
                                   uint maxFunctions,
                                   uint maxNameLen);

/** */
void* QITI_API getAddressForMangledFunctionName(const char* mangledName);

/** */
[[nodiscard]] const qiti::FunctionData* QITI_API getFunctionData(const char* demangledFunctionName);

/** */
template <auto FuncPtr>
[[nodiscard]] constexpr const qiti::FunctionData* getFunctionData()
{
    constexpr std::string_view functionName = getFunctionName<FuncPtr>();\
    return getFunctionData(functionName.data());
}

/** Internal */
[[nodiscard]] qiti::FunctionData& QITI_API getFunctionDataFromAddress(void* functionAddress);

} // namespace qiti
