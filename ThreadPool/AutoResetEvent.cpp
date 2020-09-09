#include "pch.h"
#include "AutoResetEvent.h"

namespace CustomThreading
{
    AutoResetEvent::AutoResetEvent(bool initial) : flag_(initial)
    {
    }

    void AutoResetEvent::Set()
    {
        std::lock_guard<std::mutex> _(protect_);
        flag_ = true;
        signal_.notify_all();
    }

    void AutoResetEvent::Reset()
    {
        std::lock_guard<std::mutex> _(protect_);
        flag_ = false;
    }

    bool AutoResetEvent::WaitOne()
    {
        std::unique_lock<std::mutex> unique_lock(protect_);
        while (!flag_) // prevent spurious wakeups from doing harm
            signal_.wait(unique_lock);

        flag_ = false; // waiting resets the flag
        return true;
    }
}