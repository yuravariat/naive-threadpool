#include "pch.h"
#include "SpinLock.h"

namespace CustomThreading
{
	void SpinLock::lock() noexcept {
		for (;;) {
			// Optimistically assume the lock is free on first the try
			if (!lock_.exchange(true, std::memory_order_acquire)) {
				return;
			}
			// Wait for lock to be released without generating cache misses
			while (lock_.load(std::memory_order_relaxed)) {
				// Issue X86 PAUSE or ARM YIELD instruction to reduce contention between
				// hyper-threads
				_mm_pause();
				//__builtin_ia32_pause();
			}
		}
	}

	bool SpinLock::try_lock() noexcept {
		// First do a relaxed load to check if lock is free in order to prevent
		// unnecessary cache misses if someone does while(!try_lock())
		return !lock_.load(std::memory_order_relaxed) &&
			!lock_.exchange(true, std::memory_order_acquire);
	}

	void SpinLock::unlock() noexcept {
		lock_.store(false, std::memory_order_release);
	}
}