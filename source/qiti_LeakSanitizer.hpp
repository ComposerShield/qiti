
#pragma once

#include "qiti_API.hpp"

//--------------------------------------------------------------------------

namespace qiti
{
/**
 */
class LeakSanitizer
{
public:
    /** */
    [[nodiscard]] bool QITI_API passed() noexcept;
    
    /** */
    [[nodiscard]] inline bool QITI_API failed() noexcept { return ! passed(); }
    
    /** Internal */
    QITI_API ~LeakSanitizer() noexcept;

    /** Move Constructor */
    QITI_API LeakSanitizer(LeakSanitizer&& other) noexcept;
    /** Move Operator */
    [[nodiscard]] LeakSanitizer& QITI_API operator=(LeakSanitizer&& other) noexcept;
    
private:
    /** Internal */
    QITI_API_INTERNAL LeakSanitizer() noexcept;
    
    bool failed = false;
};
} // namespace qiti
