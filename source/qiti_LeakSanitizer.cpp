
#include "qiti_LeakSanitizer.hpp"

namespace qiti
{
LeakSanitizer::LeakSanitizer() noexcept
{
    
}

LeakSanitizer::~LeakSanitizer() noexcept
{
    
}

bool LeakSanitizer::passed() noexcept
{
    return ! _failed;  // TODO: implement
}

} // namespace qiti
