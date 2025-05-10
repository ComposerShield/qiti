
#pragma once

#include "qiti_API.hpp"

#include <type_traits>

//--------------------------------------------------------------------------

namespace qiti
{
/**
 */
class ThreadSanitizer
{
public:
    /** */
    template<auto FuncPtr0, auto FuncPtr1>
    requires std::is_function_v<std::remove_pointer_t<decltype(FuncPtr0)>>
    && std::is_function_v<std::remove_pointer_t<decltype(FuncPtr1)>>
    [[nodiscard]] static ThreadSanitizer QITI_API functionsNotCalledInParallel() noexcept
    {
        return functionsNotCalledInParallel(reinterpret_cast<void*>(FuncPtr0),
                                            reinterpret_cast<void*>(FuncPtr1));
    }
    
    /** */
    [[nodiscard]] bool QITI_API passedTest() noexcept;
    
    /** Internal */
    QITI_API ~ThreadSanitizer() noexcept;

    /** Move Constructor */
    QITI_API ThreadSanitizer(ThreadSanitizer&& other) noexcept;
    /** IMove Operator */
    [[nodiscard]] ThreadSanitizer& QITI_API operator=(ThreadSanitizer&& other) noexcept;
    
    struct Impl;
    /** Internal */
    [[nodiscard]] Impl* QITI_API_INTERNAL getImpl() noexcept;
    /** Internal */
    [[nodiscard]] const Impl* QITI_API_INTERNAL getImpl() const noexcept;
    
private:
    /** Internal */
    QITI_API_INTERNAL ThreadSanitizer() noexcept;
    
    /** Internal */
    static ThreadSanitizer QITI_API functionsNotCalledInParallel(void* func0, void* func1);
    
    bool failed = false;
    
    // Stack-based pimpl idiom
    static constexpr size_t ImplSize  = 456;
    static constexpr size_t ImplAlign =  8;
    alignas(ImplAlign) unsigned char implStorage[ImplSize];
    
    ThreadSanitizer(const ThreadSanitizer&) = delete;
    ThreadSanitizer& operator=(const ThreadSanitizer&) = delete;
    
    // Prevent heap allocating this class
    void* operator new(size_t) = delete;
    void* operator new[](size_t) = delete;
};
} // namespace qiti
