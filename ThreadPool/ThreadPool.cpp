#include "pch.h"
#include "ThreadPool.h"

CustomThreading::ThreadPool::ThreadPool(unsigned int numOfThreads)
	:numberOfThreads(numOfThreads),
    forceStop(false)
{
	threads = std::vector<std::thread>();
    for (unsigned int i = 0; i < numberOfThreads; i++) {
        threads.push_back(std::thread([this] {
            this->ThreadLoop();
        }));
    }
}
CustomThreading::ThreadPool::ThreadPool(ThreadPool& copyFrom)
    : forceStop(copyFrom.forceStop), 
    numberOfThreads(copyFrom.numberOfThreads)
{
   /* threads = copyFrom.threads;
    workItems = copyFrom.workItems;*/
}
CustomThreading::ThreadPool::ThreadPool(ThreadPool&& moveFrom)
    : forceStop(moveFrom.forceStop), 
    numberOfThreads(moveFrom.numberOfThreads),
    threads(std::move(moveFrom.threads)),
    workItems(std::move(moveFrom.workItems))
{
}
CustomThreading::ThreadPool::~ThreadPool()
{
}

bool CustomThreading::ThreadPool::QuqueTask(std::shared_ptr<Task>& task)
{
    bool success = true;
    try {
        std::unique_lock<std::mutex> lock(m_mutex);
        task->m_Status = TaskStatus::WaitingToRun;
        workItems.emplace_back(task);
    }
    catch (std::exception ex) {
        success = false;
    }
    catch (...) {
        success = false;
    }

    return success;
}

void CustomThreading::ThreadPool::ThreadLoop()
{
	while (!this->forceStop)
	{
        // try inqueue
        std::shared_ptr<Task> task;
        {
            // if empty, lock
            std::unique_lock<std::mutex> lock(m_mutex);
            if (workItems.empty()) {
                cond_var.wait(lock, [&] {return !workItems.empty(); });
            }
            task = workItems.front();
            workItems.pop_front();
        }
        // if the task is invalid, it means we are asked to abort:
        if (!task->LambdaWithParams.valid()) return;

        // otherwise, run the task:
        task->m_Status = TaskStatus::Running;
        task->LambdaWithParams();
        task->m_Status = TaskStatus::RanToCompletion;
	}
}
