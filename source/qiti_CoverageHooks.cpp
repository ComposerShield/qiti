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
#include <unordered_map>
#include <stdexcept>
#include <string>
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

/** one arc = an edge in the CFG, from one block to another */
struct ArcCoverage
{
    uint32_t   from;    // the source block index
    uint32_t   to;      // the destination block index
    uint64_t   count;   // how many times that edge was taken
};

/** A little POD to hold everything we care about for one function */
struct FunctionCoverage
{
    uint32_t                 ident;     // function ID
    uint32_t                 checksum;  // function checksum
    std::vector<uint64_t>    counters;  // per-block hit counts
    std::vector<ArcCoverage> arcs;      // per-edge (branch) counts
};

/** */
static std::unordered_map<uint32_t, FunctionCoverage> QITI_API_INTERNAL parse_gcda(const char* path) noexcept(false)
{
    qiti::MallocHooks::ScopedBypassMallocHooks bypassMallocHooks;

    std::ifstream in(path, std::ios::binary);
    if (! in.is_open())
    {
        throw std::runtime_error(std::string("Failed to open .gcda file: ") + path);
    }

    uint32_t magic;
    uint32_t version;
    in.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    in.read(reinterpret_cast<char*>(&version), sizeof(version));
    if (magic != GCOV_DATA_MAGIC)
    {
        throw std::runtime_error(std::string("Not a .gcda file: ") + path);
    }

    std::unordered_map<uint32_t, FunctionCoverage> coverage;
    FunctionCoverage* currentFn = nullptr;

    while (true)
    {
        uint32_t tag;
        uint32_t len;
        in.read(reinterpret_cast<char*>(&tag), sizeof(tag));
        in.read(reinterpret_cast<char*>(&len), sizeof(len));
        if (! in)
            break; // reached the end

        if (tag == GCOV_TAG_FUNCTION)
        {
            FunctionCoverage fn;
            in.read(reinterpret_cast<char*>(&fn.ident),   sizeof(fn.ident));
            in.read(reinterpret_cast<char*>(&fn.checksum), sizeof(fn.checksum));
            auto inserted = coverage.emplace(fn.ident, std::move(fn));
            currentFn = &inserted.first->second;
        }
        else if ((tag & 0xFFFFu) == 0 && tag >= GCOV_TAG_COUNTER_BASE)
        {
            unsigned type  = (tag - GCOV_TAG_COUNTER_BASE) >> 17;
            unsigned words = len;

            if (!currentFn)
            {
                // no function context—just skip
                std::streamoff offset = static_cast<std::streamoff>(words)
                                        * static_cast<std::streamoff>(sizeof(uint32_t));
                in.seekg(offset, std::ios::cur);
            }
            else if (type == 0)
            {
                // block counters: each entry is (uint32, uint64)
                unsigned entryWords = (sizeof(uint32_t) + sizeof(uint64_t))
                                      / sizeof(uint32_t);
                unsigned n = words / entryWords;
                currentFn->counters.resize(n);
                for (unsigned i = 0; i < n; ++i)
                {
                    uint32_t idx;
                    uint64_t val;
                    in.read(reinterpret_cast<char*>(&idx), sizeof(idx));
                    in.read(reinterpret_cast<char*>(&val), sizeof(val));
                    currentFn->counters[i] = val;
                }
            }
            else if (type == 1)
            {
                // branch counters: (uint32, uint32, uint64)
                unsigned entryWords = (2*sizeof(uint32_t) + sizeof(uint64_t))
                                      / sizeof(uint32_t);
                unsigned n = words / entryWords;
                currentFn->arcs.reserve(n);
                for (unsigned i = 0; i < n; ++i)
                {
                    uint32_t from, to;
                    uint64_t cnt;
                    in.read(reinterpret_cast<char*>(&from), sizeof(from));
                    in.read(reinterpret_cast<char*>(&to),   sizeof(to));
                    in.read(reinterpret_cast<char*>(&cnt),  sizeof(cnt));
                    currentFn->arcs.push_back({ from, to, cnt });
                }
            }
            else
            {
                // some other counter type—skip it
                std::streamoff offset = static_cast<std::streamoff>(words)
                                        * static_cast<std::streamoff>(sizeof(uint32_t));
                in.seekg(offset, std::ios::cur);
            }
        }
        else
        {
            // totally unknown record—skip
            std::streamoff offset = static_cast<std::streamoff>(len)
                                    * static_cast<std::streamoff>(sizeof(uint32_t));
            in.seekg(offset, std::ios::cur);
        }
    }

    return coverage;
}

static std::unordered_map<uint32_t, std::string> QITI_API_INTERNAL parse_gcno(const char* path) noexcept(false)
{
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open())
    {
        throw std::runtime_error(std::string("Failed to open .gcno file: ") + path);
    }

    uint32_t magic;
    uint32_t version;
    uint32_t stamp;
    uint32_t support;
    in.read(reinterpret_cast<char*>(&magic),   sizeof(magic));
    in.read(reinterpret_cast<char*>(&version), sizeof(version));
    in.read(reinterpret_cast<char*>(&stamp),   sizeof(stamp));
    in.read(reinterpret_cast<char*>(&support), sizeof(support));

    std::unordered_map<uint32_t, std::string> names;

    while (true)
    {
        uint32_t tag;
        uint32_t len;
        in.read(reinterpret_cast<char*>(&tag), sizeof(tag));
        in.read(reinterpret_cast<char*>(&len), sizeof(len));
        if (!in || tag == 0)
        {
            break;
        }

        if (tag == GCOV_TAG_FUNCTION)
        {
            uint32_t ident;
            in.read(reinterpret_cast<char*>(&ident), sizeof(ident));

            // skip two checksum fields
            in.seekg(static_cast<std::streamoff>(2 * sizeof(uint32_t)), std::ios::cur);

            // read function name string
            uint32_t nameLen;
            in.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
            std::string name;
            name.resize(nameLen);
            in.read(&name[0], nameLen);
            // align to 4 bytes
            uint32_t pad = (4 - (nameLen % 4)) % 4;
            in.seekg(static_cast<std::streamoff>(pad), std::ios::cur);

            // skip source filename string
            uint32_t srcLen;
            in.read(reinterpret_cast<char*>(&srcLen), sizeof(srcLen));
            uint32_t srcPad = (4 - (srcLen % 4)) % 4;
            in.seekg(static_cast<std::streamoff>(srcLen + srcPad), std::ios::cur);

            names.emplace(ident, std::move(name));
        }
        else
        {
            std::streamoff offset = static_cast<std::streamoff>(len)
                                    * static_cast<std::streamoff>(sizeof(uint32_t));
            in.seekg(offset, std::ios::cur);
        }
    }

    return names;
}

[[maybe_unused]] static void QITI_API_INTERNAL coverageTest()
{
    qiti::MallocHooks::ScopedBypassMallocHooks bypassMallocHooks;

    // Build paths
//    std::string gcdaPath = std::string(coveragePath) + "/.../qiti_LockHooks.gcda";
//    std::string gcnoPath = gcdaPath;
//    auto pos = gcnoPath.rfind(".gcda");
//    if (pos != std::string::npos)
//    {
//        gcnoPath.replace(pos, 5, ".gcno");
//    }
    
    auto path = "/tmp/coverage/qiti/build/build/qiti_lib.build/Debug/Objects-normal/arm64/qiti_LockHooks.gcda";

    // Parse names and coverage data
    auto nameMap = parse_gcno(path);
    auto coverageMap = parse_gcda(path);

    for (auto const& entry : coverageMap)
    {
        uint32_t funcId = entry.first;
        FunctionCoverage const& fc = entry.second;

        // Print function name if available
        auto it = nameMap.find(funcId);
        if (it != nameMap.end())
        {
            std::cout << "Function Name: " << it->second << "\n";
        }
        else
        {
            std::cout << "Function ID: " << funcId << "\n";
        }

        std::cout << "  Checksum: " << fc.checksum << "\n";

        std::cout << "  Block counts:";
        for (size_t i = 0; i < fc.counters.size(); ++i)
        {
            std::cout << " [" << i << "]=" << fc.counters[i];
        }
        std::cout << "\n";

        bool allBranchesTaken = true;
        for (auto const& arc : fc.arcs)
        {
            if (arc.count == 0)
            {
                allBranchesTaken = false;
                break;
            }
        }
        std::cout << "  All branches taken: " << (allBranchesTaken ? "true" : "false") << "\n\n";
    }
}

//--------------------------------------------------------------------------

namespace qiti
{
void CoverageHooks::dump() noexcept
{
    __gcov_dump();
//    coverageTest();
}

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
