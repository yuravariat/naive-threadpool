#pragma once
#include "pch.h"

namespace CustomThreading
{
    class AutoResetEvent
    {
    public:
        explicit AutoResetEvent(bool initial = false);
        void Set();
        void Reset();
        bool WaitOne();

    private:
        AutoResetEvent(const AutoResetEvent&);
        AutoResetEvent& operator=(const AutoResetEvent&) = delete; // non-copyable
        bool flag_;
        std::mutex protect_;
        std::condition_variable signal_;
    };
}
