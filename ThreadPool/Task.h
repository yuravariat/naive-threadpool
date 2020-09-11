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
#include "AutoResetEvent.h"

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
		Task() : m_Status(TaskStatus::Created), _id(0), _runCompilteEvent(nullptr) {
			_id = _idCounter.fetch_add(1);
		}
		template<class LambdaFunc>
		Task(LambdaFunc&& func) : Task()
		{
			LambdaWithParams = std::packaged_task<void()>(std::forward<LambdaFunc>(func));
		}
		Task(Task& copyFrom) = delete;  // copy constructor deleted
		Task(Task&& moveFrom) noexcept : 
			m_Status(moveFrom.m_Status), _id(moveFrom._id), LambdaWithParams(std::move(moveFrom.LambdaWithParams)) { } // move constructor
		~Task() {
			if (_runCompilteEvent != nullptr) {
				delete _runCompilteEvent;
				_runCompilteEvent = nullptr;
			}
		}
		#pragma endregion

		template<class LambdaFunc>
		static std::shared_ptr<Task> RunVoid(LambdaFunc&& func) {
			return ApplicationThreadPool::GetInstance().QuqueTaskVoid(func);
		}

		template<class LambdaFunc, class ReturnType = std::result_of_t<LambdaFunc&()>>
		static std::shared_ptr<TTask<ReturnType>> Run(LambdaFunc&& func) {
			return ApplicationThreadPool::GetInstance().QuqueTask(func);
		}

		std::packaged_task<void()> LambdaWithParams;
		std::exception exception;

		TaskStatus GetStatus() { return m_Status; }
		unsigned long GetID() { return _id; }
		void Wait() {

			if (m_Status == TaskStatus::Created)
				throw std::exception("call run before wait");

			if ((short)this->m_Status > 4)
				return;

			// spin a little
			int spins = std::thread::hardware_concurrency() == 1 ? 1 : 40;
			for (int i = 0; i< spins; i++) {
				if (i % 5 == 0)
					std::this_thread::sleep_for(std::chrono::milliseconds(0));
				else if (i % 10 == 0)
					std::this_thread::yield();
				else if (i % 20)
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				else
					_mm_pause();

				if ((short)this->m_Status > 4)
					return;
			}
			// now the OS lock
			if (_runCompilteEvent == nullptr) {
				_runCompilteEvent = new AutoResetEvent();
			}
			// check again, while preparing the AutoResetEvent, InternalRun and PostRun maybe already finished.
			if ((short)this->m_Status > 4)
				return;

			// now wait
			_runCompilteEvent->WaitOne();
		}
		template <typename... Tasks>
		static void WaitAll(Tasks... tasks) {
			std::shared_ptr<Task> tasks_to_wait[] = { static_cast<std::shared_ptr<Task>>(tasks)... };
			unsigned int items = sizeof...(tasks);
			for (unsigned int i = 0; i < items; i++) {
				tasks_to_wait[i]->Wait();
			}
		}

	protected:
		TaskStatus m_Status;
		unsigned long _id;

	private:
		AutoResetEvent* _runCompilteEvent;
		virtual bool InternalRun() {

			if (!LambdaWithParams.valid()) 
				return false;

			LambdaWithParams();
			return true;
		}
		void PostRun() {
			if (_runCompilteEvent != nullptr)
				_runCompilteEvent->Set();
		}
		static std::atomic<unsigned long> _idCounter;
		static class _init { public: _init() { _idCounter = 1; }} _initializer;

		friend class ThreadPool;
		friend class ApplicationThreadPool;
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
		TTask(TTask& copyFrom) = delete;  // copy constructor deleted
		TTask(TTask&& moveFrom) noexcept 
			: m_Status(moveFrom.m_Status), _id(moveFrom._id), LambdaWithParams(std::move(moveFrom.LambdaWithParams)) { } // move constructor
		~TTask() {}
		// todo: make it private and expose GetResult method that will include also Wait()
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
