
#include "qiti_utils.hpp"

#include <cxxabi.h>

namespace qiti
{

std::string demangle(const char* mangledName)
{
    int status = 0;
    // abi::__cxa_demangle allocates with malloc; we wrap it in unique_ptr so it frees automatically
    std::unique_ptr<char, void(*)(void*)> result
    {
        abi::__cxa_demangle(mangledName, nullptr, nullptr, &status),
        std::free
    };
    // on success, result.get() is our demangled C-string; otherwise fall back
    return (status == 0 && result) ? result.get() : mangledName;
}

} // namespace qiti
