
#define QITI_API __attribute__((no_instrument_function))

namespace qiti
{
//using time_ns = int64_t;

struct FunctionData
{
    const char* functionNameMangled;
    const char* functionNameReal;
//    int64_t numTimesCalled = 0;
//    time_ns averageTimeSpentInFunction = 0;
    
    struct LastCallData
    {
//        std::chrono::steady_clock::time_point begin_time;
//        std::chrono::steady_clock::time_point end_time;
//        time_ns timeSpentInFunction = 0;
        
//        int32_t numHeapAllocations = 0;
    };
    LastCallData lastCallData;
};

/** demangle a GCC/Clang‐mangled name into a std::string */
void QITI_API demangle(const char* mangled_name,
                       char* demangled_name,
                       unsigned long long demangled_size);

} // namespace qiti
