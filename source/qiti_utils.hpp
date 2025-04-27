
#include <cstdint>
#include <string>

namespace qiti
{
using time_ms = int64_t;

struct FunctionData
{
    int64_t numTimesCalled = 0;
    time_ms averageTimeSpentInFunction = 0;
};

/** demangle a GCC/Clang‐mangled name into a std::string */
[[nodiscard]] std::string demangle(const char* mangledName);

FunctionData& createAndRegisterFunctionData(const std::string& demangledFunctionName);

} // namespace qiti
