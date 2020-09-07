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
		bool QuqueTask(std::shared_ptr<Task>& task);
	private:
		unsigned int numberOfThreads;
		std::vector<std::thread> threads;
		std::mutex m_mutex;
		std::condition_variable cond_var;
		bool forceStop;
		std::deque<std::shared_ptr<Task>> workItems;
		void ThreadLoop();
	};

	class ApplicationThreadPool
	{
	public:
		static ApplicationThreadPool& GetInstance() {
			static ApplicationThreadPool instance;
			return instance;
		}
		bool QuqueTask(std::shared_ptr<Task>& task) {
			return m_ThreadPool->QuqueTask(task);
		}
		ApplicationThreadPool(const ApplicationThreadPool&) = delete;
		ApplicationThreadPool& operator=(const ApplicationThreadPool&) = delete;
	private:
		ApplicationThreadPool() : m_ThreadPool(nullptr) { m_ThreadPool = new ThreadPool(); };
		~ApplicationThreadPool() {
			if (m_ThreadPool != nullptr) {
				delete m_ThreadPool;
				m_ThreadPool = nullptr;
			}
		}
		ThreadPool* m_ThreadPool;
	};
}

