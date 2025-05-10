
#pragma once

//--------------------------------------------------------------------------

namespace qiti
{
/**
 Abtracts a type and its history of use
 */
class ThreadSanitizer
{
public:
    /** */
    template<auto Func0, auto Func1>
    requires std::is_function_v<std::remove_pointer_t<decltype(FuncPtr0)>>
    && std::is_function_v<std::remove_pointer_t<decltype(FuncPtr1)>>
    [[nodiscard]] ThreadSanitizer QITI_API functionsNotCalledInParallel() const noexcept
    {
        functionsNotCalledInParallel(reinterpret_cast<void*>(FuncPtr0),
                                     reinterpret_cast<void*>(FuncPtr1));
    }
    
    /** */
    [[nodiscard]] bool QITI_API passedTest();
    
    
    struct Impl;
    /** Internal */
    [[nodiscard]] Impl* QITI_API_INTERNAL getImpl() noexcept;
    /** Internal */
    [[nodiscard]] const Impl* QITI_API_INTERNAL getImpl() const noexcept;
    
    /** Internal Move Constructor */
    QITI_API_INTERNAL TypeData(TypeData&& other) noexcept;
    /** Internal Move Operator */
    [[nodiscard]] TypeData& QITI_API_INTERNAL operator=(TypeData&& other) noexcept;
    
private:
    /** Internal */
    QITI_API_INTERNAL ThreadSanitizer() noexcept;
    /** Internal */
    QITI_API_INTERNAL ~ThreadSanitizer() noexcept;
    
    /** */
    void functionsNotCalledInParallel(void* func0, void* func1);
    
    // Stack-based pimpl idiom
    static constexpr size_t ImplSize  = 456;
    static constexpr size_t ImplAlign =  8;
    alignas(ImplAlign) unsigned char implStorage[ImplSize];
    
    TypeData(const TypeData&) = delete;
    TypeData& operator=(const TypeData&) = delete;
    
    // Prevent heap allocating this class
    void* operator new(size_t) = delete;
    void* operator new[](size_t) = delete;
};
} // namespace qiti
