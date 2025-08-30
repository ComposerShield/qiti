
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_include.hpp
 *
 * @author   Adam Shield
 * @date     2025-05-16
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#pragma once

// Macros
#include "../source/qiti_API.hpp"

// Classes
#include "../source/qiti_FunctionData.hpp"
#include "../source/qiti_FunctionCallData.hpp"
#include "../source/qiti_ScopedQitiTest.hpp"

#ifdef QITI_ENABLE_CLANG_THREAD_SANITIZER
#include "../source/qiti_ThreadSanitizer.hpp"
#endif

//--------------------------------------------------------------------------
