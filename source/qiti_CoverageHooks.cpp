
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_LockData.hpp
 *
 * @author   Adam Shield
 * @date     2025-05-25
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#include "qiti_CoverageHooks.hpp"

#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <iostream>

//--------------------------------------------------------------------------

static constexpr const char* coveragePath = "/tmp/coverage";

extern "C"
{
    void __gcov_reset(); // defined when linking with --coverage
    void __gcov_dump (); // defined when linking with --coverage
}

extern bool isQitiTestRunning() noexcept;

__attribute__((constructor)) // force environment variables at startup
static void QITI_API_INTERNAL setup_gcov_prefix()
{
    (void)setenv("GCOV_PREFIX", coveragePath, 1);
    (void)setenv("GCOV_PREFIX_STRIP", "3", 1); // limit the number of nested directories inside "coverage"
}

//--------------------------------------------------------------------------

namespace qiti
{
void CoverageHooks::dump() noexcept { __gcov_dump(); }

void CoverageHooks::reset() noexcept(false)
{
    static constexpr auto* dir = coveragePath;
    try
    {
        if (std::filesystem::exists(dir) && std::filesystem::is_directory(dir))
        {
            std::uintmax_t removed = std::filesystem::remove_all(dir);
            //std::cout << "Removed " << removed << " files/directories from " << dir << "\n";
        }
        else
        {
            //std::cout << dir << " does not exist or is not a directory.\n";
        }
    }
    catch (const std::filesystem::filesystem_error& ex)
    {
        //std::cerr << "Error removing directory: " << ex.what() << "\n";
        assert(false);
    }
    
    __gcov_reset(); // delete all local counters
}
} // namespace qiti
