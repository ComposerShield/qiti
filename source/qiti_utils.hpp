
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
class FunctionData
{
public:
    class FunctionCallData
    {
    public:
        unsigned long long QITI_API getNumHeapAllocations() const noexcept;
        
        /** Internal */
        QITI_API_INTERNAL FunctionCallData();
        /** Internal */
        QITI_API_INTERNAL ~FunctionCallData();
        
        class Impl;
        /** Internal */
        Impl* QITI_API_INTERNAL getImpl() const noexcept;
        
        /** Internal */
        void QITI_API_INTERNAL reset() noexcept;
        
        /** Internal Move Constructor */
        QITI_API_INTERNAL FunctionCallData(FunctionCallData&& other);
        /** Internal Move Operator */
        FunctionCallData& QITI_API_INTERNAL operator=(FunctionCallData&& other) noexcept;
        
    private:
        Impl* impl;
        
        FunctionCallData(const FunctionCallData&) = delete;
        FunctionCallData& operator=(const FunctionCallData&) = delete;
    };
    
    /** */
    const char* QITI_API getFunctionName() const noexcept;
    
    /** */
    unsigned long long QITI_API getNumTimesCalled() const noexcept;

    /** */
    const FunctionCallData* QITI_API getLastFunctionCall() const noexcept;
    
    /** Internal */
    QITI_API_INTERNAL FunctionData(void* functionAddress);
    /** Internal */
    QITI_API_INTERNAL ~FunctionData();
    
    class Impl;
    /** Internal */
    Impl* QITI_API_INTERNAL getImpl() const noexcept;
    
    /** Internal Move Constructor */
    QITI_API_INTERNAL FunctionData(FunctionData&& other);
    /** Internal Move Operator */
    FunctionData& QITI_API_INTERNAL operator=(FunctionData&& other) noexcept;
    
private:
    Impl* impl;
    
    FunctionData(const FunctionData&) = delete;
    FunctionData& operator=(const FunctionData&) = delete;
};

/** demangle a GCC/Clang‐mangled name into a std::string */
void QITI_API demangle(const char* mangled_name,
                       char* demangled_name,
                       unsigned long long demangled_size);


/** */
void QITI_API shutdown();

/** */
void QITI_API printAllKnownFunctions();

/** */
[[nodiscard]] const qiti::FunctionData* QITI_API getFunctionData(const char* demangledFunctionName);

/** Internal */
[[nodiscard]] qiti::FunctionData& QITI_API getFunctionData(void* functionAddress);

} // namespace qiti
