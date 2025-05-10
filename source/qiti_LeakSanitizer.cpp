
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
    return ! failed;  // TODO: implement
}

} // namespace qiti
