#pragma once
#include "pch.h"

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
		Task();
		template<class LambdaFunc, class ReturnType = std::result_of_t<LambdaFunc& ()>>
		Task(LambdaFunc&& func) 
			:m_Status(TaskStatus::Created)
		{
			LambdaWithParams = std::packaged_task<ReturnType()>(std::forward<LambdaFunc>(func));
		}
		template<class LambdaFunc, class ReturnType = std::result_of_t<LambdaFunc& ()>>
		static std::shared_ptr<Task> Run(LambdaFunc&& func) {
			auto task = std::make_shared<Task>(func);
			//ApplicationThreadPool::GetInstance().QuqueTask(task);
			return task;
		}
		~Task();
		Task(Task& copyFrom); // copy constructor
		Task(Task&& moveFrom); // move constructor

		TaskStatus GetStatus() { return m_Status; }

	private:
		TaskStatus m_Status;
		std::packaged_task<void()> LambdaWithParams;

		friend class ThreadPool;
	};
}

