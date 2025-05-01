
#include "qiti_profile.hpp"

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

//--------------------------------------------------------------------------

namespace qiti
{
namespace profile
{
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
} // namespace profile
} // namespace qiti
