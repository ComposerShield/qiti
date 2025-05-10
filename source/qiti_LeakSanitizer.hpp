
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
    QITI_API LeakSanitizer() noexcept;
    QITI_API ~LeakSanitizer() noexcept;
    
    /** */
    [[nodiscard]] bool QITI_API passed() noexcept;
    
    /** */
    [[nodiscard]] inline bool QITI_API failed() noexcept { return ! passed(); }

    /** Move Constructor */
    QITI_API LeakSanitizer(LeakSanitizer&& other) noexcept;
    /** Move Assignment */
    [[nodiscard]] LeakSanitizer& QITI_API operator=(LeakSanitizer&& other) noexcept;
    
private:
    bool _failed = false;
    
    /** Copy Constructor (deleted) */
    LeakSanitizer(const LeakSanitizer&) = delete;
    /** Copy Assignment (deleted) */
    LeakSanitizer& operator=(const LeakSanitizer&) = delete;
};
} // namespace qiti
