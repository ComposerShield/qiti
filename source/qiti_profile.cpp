
#include "qiti_profile.hpp"

#include <mutex>
#include <unordered_set>

//--------------------------------------------------------------------------

std::unordered_set<void*> g_functionsToProfile;
bool g_profileAllFunctions = false;

struct Init_g_functionsToProfile
{
    Init_g_functionsToProfile()
    {
        g_functionsToProfile.reserve(256);
    }
};
static const Init_g_functionsToProfile init_g_functionsToProfile;

extern std::recursive_mutex qiti_global_lock;

//--------------------------------------------------------------------------

namespace qiti
{
namespace profile
{
void resetProfiling() noexcept
{
    // Prevent any qiti work while we disable profiling
    std::scoped_lock<std::recursive_mutex> lock(qiti_global_lock);
    
    g_functionsToProfile.clear();
    g_profileAllFunctions = false;
}

void beginProfilingFunction(void* functionAddress) noexcept
{
    g_functionsToProfile.insert(functionAddress);
}

void endProfilingFunction(void* functionAddress) noexcept
{
    g_functionsToProfile.erase(functionAddress);
}

void beginProfilingAllFunctions() noexcept
{
    g_profileAllFunctions = true;
}

void endProfilingAllFunctions() noexcept
{
    g_profileAllFunctions = false;
}

bool shouldProfileFunction(void* funcAddress) noexcept
{
    return g_profileAllFunctions || g_functionsToProfile.contains(funcAddress);
}
} // namespace profile
} // namespace qiti
