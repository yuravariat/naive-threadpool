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
	/// <summary>
	/// Task life-time cycle statuses
	/// </summary>
	/////////////////////////////////////////////////////////////////////////////////////////////////////

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

	/// <summary>
	/// Base void task
	/// </summary>
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	class Task
	{
	public:

		#pragma region Constr
		Task() : m_Status(TaskStatus::Created), _id(0) {
			_id = _idCounter.fetch_add(1);
		}
		template<class LambdaFunc>
		Task(LambdaFunc&& func) : Task()
		{
			LambdaWithParams = std::packaged_task<void()>(std::forward<LambdaFunc>(func));
		}
		Task(Task& copyFrom) : m_Status(copyFrom.m_Status), _id(copyFrom._id), LambdaWithParams(&copyFrom.LambdaWithParams){} // copy constructor
		Task(Task&& moveFrom) noexcept : m_Status(moveFrom.m_Status), _id(moveFrom._id), LambdaWithParams(std::move(moveFrom.LambdaWithParams)) { } // move constructor
		~Task() {}
		#pragma endregion

		template<class LambdaFunc>
		static std::shared_ptr<Task> RunVoid(LambdaFunc&& func) {
			return ApplicationThreadPool::GetInstance().QuqueTaskVoid(func);
		}

		template<class LambdaFunc, class ReturnType = std::result_of_t<LambdaFunc&()>>
		static std::shared_ptr<TTask<ReturnType>> Run(LambdaFunc&& func) {
			return ApplicationThreadPool::GetInstance().QuqueTask(func);
		}

		TaskStatus GetStatus() { return m_Status; }
		unsigned long GetID() { return _id; }
		std::packaged_task<void()> LambdaWithParams;

	protected:
		TaskStatus m_Status;
		unsigned long _id;

	private:
		virtual bool InternalRun() { 
			if (!LambdaWithParams.valid()) return false;
			LambdaWithParams();
			return true;
		}
		static std::atomic<unsigned long> _idCounter;
		static class _init { public: _init() { _idCounter = 1; }} _initializer;

		friend class ThreadPool;
	};

	/// <summary>
	/// Task with results
	/// </summary>
	/// <typeparam name="T">Return type of the task</typeparam>
	/////////////////////////////////////////////////////////////////////////////////////////////////////

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
