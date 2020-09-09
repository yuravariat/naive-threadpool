#pragma once
#include "pch.h"
namespace CustomThreading
{
	template <typename T>
	class TTask;
	class Task;
	class ApplicationThreadPool;
}
#include "ThreadPool.h"

namespace CustomThreading
{
	enum class TaskStatus : short {
		Created = 0,
		WaitingForActivation = 1,
		WaitingToRun = 2,
		Running = 3,
		WaitingForChildrenToComplete = 4,
		RanToCompletion = 5,
		Canceled = 6,
		Faulted = 7
	};



	class Task
	{
	public:
		Task() : m_Status(TaskStatus::Created) {}
		~Task() {}
		template<class LambdaFunc>
		Task(LambdaFunc&& func)
		{
			LambdaWithParams = std::packaged_task<void()>(std::forward<LambdaFunc>(func));
		}
		Task(Task& copyFrom) : m_Status(copyFrom.m_Status){} // copy constructor
		Task(Task&& moveFrom) : m_Status(moveFrom.m_Status) {} // move constructor

		template<class LambdaFunc>
		static std::shared_ptr<Task> RunVoid(LambdaFunc&& func) {
			return ApplicationThreadPool::GetInstance().QuqueTaskVoid(func);
		}

		template<class LambdaFunc, class ReturnType = std::result_of_t<LambdaFunc&()>>
		static std::shared_ptr<TTask<ReturnType>> Run(LambdaFunc&& func) {
			return ApplicationThreadPool::GetInstance().QuqueTask(func);
		}

		TaskStatus GetStatus() { return m_Status; }
		std::packaged_task<void()> LambdaWithParams;
	protected:
		TaskStatus m_Status;

	private:
		virtual bool InternalRun() { 
			if (!LambdaWithParams.valid()) return false;
			LambdaWithParams();
			return true;
		}
		friend class ThreadPool;
	};



	template <typename T>
	class TTask : public Task {
	public:
		TTask() {}
		template<class LambdaFunc>
		TTask(LambdaFunc&& func)
		{
			LambdaWithParams = std::packaged_task<T()>(std::forward<LambdaFunc>(func));
		}
		~TTask() {}
		T Result;
	protected:
		std::packaged_task<T()> LambdaWithParams;
	private:
		bool InternalRun() override
		{
			if (!LambdaWithParams.valid()) return false;

			auto result = LambdaWithParams.get_future();
			LambdaWithParams();
			Result = result.get();
			return true;
		}
		friend class ThreadPool;
	};
}

