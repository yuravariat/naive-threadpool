#pragma once
#include "ThreadPool.h"

namespace CustomThreading
{
	class ApplicationThreadPool
	{
		static ThreadPool& GetInstance() {
			static ThreadPool instance = ThreadPool();
			return instance;
		}
		private:
			ApplicationThreadPool() = default;
			~ApplicationThreadPool() = default;
			ApplicationThreadPool(const ApplicationThreadPool&) = delete;
			ApplicationThreadPool& operator=(const ApplicationThreadPool&) = delete;
	};
}
