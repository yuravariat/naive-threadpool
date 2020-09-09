#pragma once
#include "pch.h"
#include <Windows.h>

namespace CustomThreading
{
    struct SpinLock {

        std::atomic<bool> lock_ = { 0 };

        void lock() noexcept;

        bool try_lock() noexcept;

        void unlock() noexcept;
    };
}