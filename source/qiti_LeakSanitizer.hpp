
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
    [[nodiscard]] bool QITI_API passedTest() noexcept;
    
    /** Internal */
    QITI_API ~LeakSanitizer() noexcept;

    /** Move Constructor */
    QITI_API LeakSanitizer(LeakSanitizer&& other) noexcept;
    /** IMove Operator */
    [[nodiscard]] LeakSanitizer& QITI_API operator=(LeakSanitizer&& other) noexcept;
    
private:
    /** Internal */
    QITI_API_INTERNAL LeakSanitizer() noexcept;
    
    bool failed = false;
};
} // namespace qiti
