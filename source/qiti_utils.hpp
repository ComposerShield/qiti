
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
    const char* QITI_API getFunctionName() const noexcept;
    
    
    
    /** Internal */
    QITI_API_INTERNAL FunctionData(void* functionAddress);
    /** Internal */
    QITI_API_INTERNAL ~FunctionData();
    
    class FunctionDataImpl;
    /** Internal */
    FunctionDataImpl* QITI_API_INTERNAL getImpl() const;
    
private:
    FunctionDataImpl* impl;
};

/** demangle a GCC/Clang‐mangled name into a std::string */
void QITI_API demangle(const char* mangled_name,
                       char* demangled_name,
                       unsigned long long demangled_size);


/** */
void QITI_API shutdown();

// User
[[nodiscard]] const qiti::FunctionData* QITI_API getFunctionData(const char* demangledFunctionName);

// Internal
[[nodiscard]] qiti::FunctionData& QITI_API getFunctionData(void* functionAddress);

} // namespace qiti
