#pragma once
#include "pch.h"
#include "Task.h"

namespace CustomThreading
{
	class ThreadPool
	{
	public:
		ThreadPool(unsigned int numOfThreads = std::thread::hardware_concurrency());
		ThreadPool(ThreadPool& copyFrom); // copy constructor
		ThreadPool(ThreadPool&& moveFrom); // move constructor
		~ThreadPool();
		bool QuqueTask(Task* task);
	private:
		unsigned int numberOfThreads;
		std::vector<std::thread> threads;
		std::mutex m_mutex;
		std::condition_variable cond_var;
		bool forceStop;
		std::deque<Task*> workItems;
		void ThreadLoop();
	};
}

