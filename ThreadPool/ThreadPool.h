#pragma once
#include "pch.h"
#include "Task.h"

class TaskStatus;

namespace CustomThreading
{
	/// <summary>
	/// Staic thread pool of the application
	/// </summary>
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	class ApplicationThreadPool
	{
	public:
		static ApplicationThreadPool& GetInstance() {
			static ApplicationThreadPool instance;
			return instance;
		}
		template<class LambdaFunc>
		std::shared_ptr<Task> QuqueTaskVoid(LambdaFunc&& func) {
			return m_ThreadPool->QuqueTaskVoid(func);
		}

		template<class LambdaFunc, class ReturnType = std::result_of_t<LambdaFunc& ()>>
		std::shared_ptr<TTask<ReturnType>> QuqueTask(LambdaFunc&& func) {
			return m_ThreadPool->QuqueTask(func);
		}
		ApplicationThreadPool(const ApplicationThreadPool&) = delete;
		ApplicationThreadPool& operator=(const ApplicationThreadPool&) = delete;
	private:
		class ThreadPool;
		ApplicationThreadPool() : m_ThreadPool(nullptr) { m_ThreadPool = new ThreadPool(); };
		~ApplicationThreadPool() {
			if (m_ThreadPool != nullptr) {
				delete m_ThreadPool;
				m_ThreadPool = nullptr;
			}
		}
		ThreadPool* m_ThreadPool;


		/// <summary>
		/// Thread pool class
		/// </summary>
		/////////////////////////////////////////////////////////////////////////////////////////////////////
		class ThreadPool
		{
		public:
			ThreadPool(unsigned int numOfThreads = std::thread::hardware_concurrency());
			ThreadPool(ThreadPool& copyFrom); // copy constructor
			ThreadPool(ThreadPool&& moveFrom); // move constructor
			~ThreadPool();
			template<class LambdaFunc>
			std::shared_ptr<Task> QuqueTaskVoid(LambdaFunc&& func)
			{
				std::shared_ptr<Task> task = std::make_shared<Task>(func);
				try {
					std::unique_lock<std::mutex> lock(m_mutex);
					task->m_Status = TaskStatus::WaitingToRun;
					workItems.emplace_back(task);
					return task;
				}
				catch (std::exception ex) {
				}
				catch (...) {
				}
				return nullptr;
			}
			template<class LambdaFunc, class ReturnType = std::result_of_t<LambdaFunc& ()>>
			std::shared_ptr<TTask<ReturnType>> QuqueTask(LambdaFunc&& func)
			{
				std::shared_ptr<TTask<ReturnType>> task = std::make_shared<TTask<ReturnType>>(func);
				try {
					std::unique_lock<std::mutex> lock(m_mutex);
					task->m_Status = TaskStatus::WaitingToRun;
					workItems.emplace_back(task);
					return task;
				}
				catch (std::exception ex) {
				}
				catch (...) {
				}
				return nullptr;
			}
		private:
			unsigned int numberOfThreads;
			std::vector<std::thread> threads;
			std::mutex m_mutex;
			std::condition_variable cond_var;
			bool forceStop;
			std::deque<std::shared_ptr<Task>> workItems;
			void ThreadLoop();
		};
	};
}

