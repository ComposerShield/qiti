
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
#include <string_view>

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
    // 1) find the last '=' in the "[FuncPtr = …]" part
    auto eq = full.rfind('=');
    if (eq == std::string_view::npos) return {};

    // 2) move past '=' then skip spaces and '&'
    auto start = eq + 1;
    while (start < full.size() && (full[start] == ' ' || full[start] == '&'))
        ++start;

    // 3) find the last ']' which closes the bracketed section
    auto end = full.rfind(']');
    if (end == std::string_view::npos || end < start)
        return {};     // unexpected format

    // 4) return exactly the characters in between
    return full.substr(start, end - start);

#elif defined(_MSC_VER)
    // similar logic with __FUNCSIG__
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
[[nodiscard]] const qiti::FunctionData* QITI_API getFunctionData()
{
    static constexpr std::string_view functionName = getFunctionName<FuncPtr>();
    static constexpr std::string_view appendText = "()";
    constexpr auto output = []
    {
        std::array<char, functionName.size() + appendText.size() + 1> buf{}; // each value initialized to '\0'
        // copy from functionName
        for (size_t i = 0; i < functionName.size(); ++i)
            buf[i] = functionName[i];
        // copy from appendText
        for (size_t i = 0; i < appendText.size(); ++i)
            buf[i + functionName.size()] = appendText[i];
        // buf[Sv.size()] is already '\0'
        return buf;
    }();
    
    return getFunctionData(output.data());
}

/** Internal */
[[nodiscard]] qiti::FunctionData& QITI_API getFunctionDataFromAddress(void* functionAddress);

} // namespace qiti
