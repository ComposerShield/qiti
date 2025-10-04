
/******************************************************************************
 * Class-specific operator new/delete overrides
 ******************************************************************************/

class AudioBuffer
{
public:
    AudioBuffer();
    ~AudioBuffer();
    
    static void* operator new(std::size_t size);
    static void operator delete(void* ptr) noexcept;
    
    static void* operator new[](std::size_t size);
    static void operator delete[](void* ptr) noexcept;
    
    // etc.
};
