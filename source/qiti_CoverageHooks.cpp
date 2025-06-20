
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

#include "qiti_MallocHooks.hpp"

#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>

//--------------------------------------------------------------------------

static constexpr const char* coveragePath = "/tmp/coverage";

extern "C"
{
    void __gcov_reset(); // defined when linking with --coverage
    void __gcov_dump (); // defined when linking with --coverage

    struct gcov_info* read_gcda_file(const char* filename);
}

extern bool isQitiTestRunning() noexcept;

__attribute__((constructor)) // force environment variables at startup
static void QITI_API_INTERNAL setup_gcov_prefix()
{
    (void)setenv("GCOV_PREFIX", coveragePath, 1);
    (void)setenv("GCOV_PREFIX_STRIP", "3", 1); // limit the number of nested directories inside "coverage"
}

/** File magic number used to identify a .gcda file: ASCII "gcda" (0x67='g',0x63='c',0x64='d',0x61='a'). */
static constexpr auto GCOV_DATA_MAGIC       = 0x67636461;

/** Tag marking the start of a function record in the .gcda data stream. */
static constexpr auto GCOV_TAG_FUNCTION     = 0x01000000;

/**
 Base tag value for counter records; actual counter tags are formed by
 GCOV_TAG_COUNTER_BASE + (counter_index << 17), indicating how many counters follow.
 */
static constexpr auto GCOV_TAG_COUNTER_BASE = 0x01a10000;

/** */
[[maybe_unused]] static void QITI_API_INTERNAL parse_gcda(const char* path)
{
    qiti::MallocHooks::ScopedBypassMallocHooks bypassMallocHooks;
    
    std::ifstream in(path, std::ios::binary);
    uint32_t magic, version;
    in.read(reinterpret_cast<char*>(&magic), 4);
    in.read(reinterpret_cast<char*>(&version), 4);
    if (magic != GCOV_DATA_MAGIC)
        throw std::runtime_error("Not a .gcda file");
    while (true)
    {
        uint32_t tag, len;
        in.read(reinterpret_cast<char*>(&tag), 4);
        in.read(reinterpret_cast<char*>(&len), 4);
        if (!in) break;
        if (tag == GCOV_TAG_FUNCTION)
        {
            uint32_t ident;
            uint32_t checksum;
            in.read(reinterpret_cast<char*>(&ident),4);
            in.read(reinterpret_cast<char*>(&checksum),4);
            // …
        }
        else if ((tag & 0xFFF80000) == GCOV_TAG_COUNTER_BASE)
        {
            unsigned cnt = (tag - GCOV_TAG_COUNTER_BASE) >> 17;
            for (unsigned i = 0; i < cnt; ++i)
            {
                uint32_t idx;
                uint32_t val;
                in.read(reinterpret_cast<char*>(&idx),4);
                in.read(reinterpret_cast<char*>(&val),4);
                // …
            }
        }
        else
        {
            in.seekg(len * 4, std::ios::cur);
        }
    }
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
            [[maybe_unused]] std::uintmax_t removed = std::filesystem::remove_all(dir);
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
