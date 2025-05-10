
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
    [[nodiscard]] bool QITI_API passed() noexcept;
    
    /** */
    [[nodiscard]] inline bool QITI_API failed() noexcept { return ! passed(); }
    
    /** Internal */
    QITI_API ~ThreadSanitizer() noexcept;

    /** Move Constructor */
    QITI_API ThreadSanitizer(ThreadSanitizer&& other) noexcept;
    /** Move Assignment */
    [[nodiscard]] ThreadSanitizer& QITI_API operator=(ThreadSanitizer&& other) noexcept;
    
private:
    bool _failed = false;
    
    /** Internal */
    QITI_API_INTERNAL ThreadSanitizer() noexcept;
    
    /** Internal */
    static ThreadSanitizer QITI_API functionsNotCalledInParallel(void* func0, void* func1);
    
    /** Copy Constructor (deleted) */
    ThreadSanitizer(const ThreadSanitizer&) = delete;
    /** Copy Assignment (deleted) */
    ThreadSanitizer& operator=(const ThreadSanitizer&) = delete;
};
} // namespace qiti
