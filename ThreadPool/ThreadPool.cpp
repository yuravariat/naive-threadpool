#include "pch.h"
#include "ThreadPool.h"

CustomThreading::ApplicationThreadPool::ThreadPool::ThreadPool(unsigned int numOfThreads)
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
CustomThreading::ApplicationThreadPool::ThreadPool::ThreadPool(ThreadPool& copyFrom)
    : forceStop(copyFrom.forceStop), 
    numberOfThreads(copyFrom.numberOfThreads)
{
   /* threads = copyFrom.threads;
    workItems = copyFrom.workItems;*/
}
CustomThreading::ApplicationThreadPool::ThreadPool::ThreadPool(ThreadPool&& moveFrom)
    : forceStop(moveFrom.forceStop), 
    numberOfThreads(moveFrom.numberOfThreads),
    threads(std::move(moveFrom.threads)),
    workItems(std::move(moveFrom.workItems))
{
}
CustomThreading::ApplicationThreadPool::ThreadPool::~ThreadPool()
{
    forceStop = true;
    for (unsigned int i = 0; i < numberOfThreads; i++) {
        threads[i].detach();
    }
    threads.clear();
}

void CustomThreading::ApplicationThreadPool::ThreadPool::ThreadLoop()
{
	while (!this->forceStop)
	{
        // try inqueue
        std::shared_ptr<Task> task;
        {
            // if empty, lock
            std::unique_lock<std::mutex> lock(m_mutex);
            if (workItems.empty()) {
                cond_var.wait(lock, [&] {return !workItems.empty() || this->forceStop; });
            }
            if (this->forceStop) {
                break;
            }
            task = workItems.front();
            workItems.pop_front();
        }
        task->m_Status = TaskStatus::Running;

        try {
            if (task.get()->InternalRun()) {
                task->m_Status = TaskStatus::RanToCompletion;
            }
            else {
                task->m_Status = TaskStatus::Faulted;
                throw std::exception("task function not valid");
            }
        }
        catch (...) {
            task->m_Status = TaskStatus::Faulted;
            task->exception_ptr = std::current_exception();
        }
        try {
            task->PostRun();
        }
        catch (...) {}
	}
}
