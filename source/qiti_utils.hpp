
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

namespace qiti
{
using uint = unsigned long long;

class FunctionCallData
{
public:
    /** */
    [[nodiscard]] uint QITI_API getNumHeapAllocations() const noexcept;
    
    /** Internal */
    QITI_API_INTERNAL FunctionCallData();
    /** */
    QITI_API ~FunctionCallData();
    
    class Impl;
    /** Internal */
    [[nodiscard]] Impl* QITI_API_INTERNAL getImpl() const noexcept;
    
    /** Internal */
    void QITI_API_INTERNAL reset() noexcept;
    
    /** Internal Move Constructor */
    QITI_API_INTERNAL FunctionCallData(FunctionCallData&& other);
    /** Internal Move Operator */
    [[nodiscard]] FunctionCallData& QITI_API_INTERNAL operator=(FunctionCallData&& other) noexcept;
    /** Internal Copy Constructor */
    FunctionCallData(const FunctionCallData&);
    /** Internal Move Operator */
    FunctionCallData operator=(const FunctionCallData&);
    
private:
    Impl* impl;
};

class FunctionData
{
public:
    /** */
    [[nodiscard]] const char* QITI_API getFunctionName() const noexcept;
    
    /** */
    [[nodiscard]] uint QITI_API getNumTimesCalled() const noexcept;

    /** */
    [[nodiscard]] FunctionCallData QITI_API getLastFunctionCall() const noexcept;
    
    /** Internal */
    QITI_API_INTERNAL FunctionData(void* functionAddress);
    /** Internal */
    QITI_API_INTERNAL ~FunctionData();
    
    class Impl;
    /** Internal */
    [[nodiscard]] Impl* QITI_API_INTERNAL getImpl() const noexcept;
    
    /** Internal Move Constructor */
    QITI_API_INTERNAL FunctionData(FunctionData&& other);
    /** Internal Move Operator */
    [[nodiscard]] FunctionData& QITI_API_INTERNAL operator=(FunctionData&& other) noexcept;
    
private:
    Impl* impl;
    
    FunctionData(const FunctionData&) = delete;
    FunctionData& operator=(const FunctionData&) = delete;
};


/** demangle a GCC/Clang‐mangled name into a std::string */
void QITI_API demangle(const char* mangled_name,
                       char* demangled_name,
                       uint demangled_size);


/** */
void QITI_API shutdown();

/** */
char** QITI_API getAllKnownFunctions();

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

/** Internal */
[[nodiscard]] qiti::FunctionData& QITI_API getFunctionData(void* functionAddress);

} // namespace qiti
