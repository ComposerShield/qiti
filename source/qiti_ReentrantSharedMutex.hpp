
// Copyright (c) 2025 Adam Shield
// SPDX-License-Identifier: MIT

#pragma once

#include "qiti_API.hpp"

#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <stdexcept>

namespace qiti
{
/**
 High-performance reentrant shared mutex:
 Recursive exclusive and shared locking by the same thread
 Shared-reader / single-writer semantics
 Uses thread-local counters and atomics to minimize global locking
 */
class ReentrantSharedMutex
{
public:
    ReentrantSharedMutex() = default;
    ~ReentrantSharedMutex() = default;
    
    // Exclusive (writer) lock
    void lock()
    {
        if (exclusive_recursion_ > 0)
        {
            // Fast path: already the writer
            ++exclusive_recursion_;
            return;
        }
        std::unique_lock<std::mutex> lk(mutex_);
        ++waiting_writers_;
        writers_cv_.wait(lk, [&]()
                         {
            return writer_id_ == std::thread::id() && active_readers_.load(std::memory_order_acquire) == 0;
        });
        --waiting_writers_;
        writer_id_ = std::this_thread::get_id();
        exclusive_recursion_ = 1;
    }
    
    /** */
    bool try_lock()
    {
        if (exclusive_recursion_ > 0)
        {
            // Fast path: already the writer
            ++exclusive_recursion_;
            return true;
        }
        std::unique_lock<std::mutex> lk(mutex_);
        if (writer_id_ != std::thread::id() || active_readers_.load(std::memory_order_acquire) > 0)
        {
            return false;
        }
        writer_id_ = std::this_thread::get_id();
        exclusive_recursion_ = 1;
        return true;
    }
    
    /** */
    void unlock()
    {
        if (exclusive_recursion_ == 0 || writer_id_ != std::this_thread::get_id())
        {
            throw std::runtime_error("unlock() called without owning exclusive lock");
        }
        --exclusive_recursion_;
        if (exclusive_recursion_ == 0)
        {
            writer_id_ = std::thread::id();
            if (waiting_writers_ > 0) {
                writers_cv_.notify_one();
            } else {
                readers_cv_.notify_all();
            }
        }
    }
    
    /** Shared (reader) lock */
    void lock_shared()
    {
        if (exclusive_recursion_ > 0)
        {
            // Writer can acquire shared lock
            ++shared_recursion_;
            active_readers_.fetch_add(1, std::memory_order_relaxed);
            return;
        }
        if (shared_recursion_ > 0)
        {
            // Recursive shared lock
            ++shared_recursion_;
            active_readers_.fetch_add(1, std::memory_order_relaxed);
            return;
        }
        std::unique_lock<std::mutex> lk(mutex_);
        readers_cv_.wait(lk, [&]()
                         {
            return writer_id_ == std::thread::id() && waiting_writers_ == 0;
        });
        ++shared_recursion_;
        active_readers_.fetch_add(1, std::memory_order_relaxed);
    }
    
    /** */
    bool try_lock_shared()
    {
        if (exclusive_recursion_ > 0)
        {
            // Writer can acquire shared lock
            ++shared_recursion_;
            active_readers_.fetch_add(1, std::memory_order_relaxed);
            return true;
        }
        if (shared_recursion_ > 0)
        {
            // Recursive shared lock
            ++shared_recursion_;
            active_readers_.fetch_add(1, std::memory_order_relaxed);
            return true;
        }
        std::unique_lock<std::mutex> lk(mutex_);
        if (writer_id_ != std::thread::id() || waiting_writers_ > 0)
        {
            return false;
        }
        ++shared_recursion_;
        active_readers_.fetch_add(1, std::memory_order_relaxed);
        return true;
    }
    
    /** */
    void unlock_shared()
    {
        if (shared_recursion_ == 0)
        {
            throw std::runtime_error("unlock_shared() called without owning shared lock");
        }
        --shared_recursion_;
        auto prev = active_readers_.fetch_sub(1, std::memory_order_acq_rel);
        if (prev == 1)
        {
            std::lock_guard<std::mutex> lk(mutex_);
            if (waiting_writers_ > 0) {
                writers_cv_.notify_one();
            }
        }
    }
    
private:
    std::mutex mutex_;
    std::condition_variable readers_cv_;
    std::condition_variable writers_cv_;
    std::atomic<unsigned int> active_readers_{0};
    unsigned int waiting_writers_{0};
    std::thread::id writer_id_;
    inline static thread_local unsigned int exclusive_recursion_ = 0;
    inline static thread_local unsigned int shared_recursion_ = 0;
};
} // namespace qiti
