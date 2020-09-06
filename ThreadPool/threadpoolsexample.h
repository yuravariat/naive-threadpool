#pragma once
#include "pch.h"

struct tasks {
    // the mutex, condition variable and deque form a single
    // thread-safe triggered queue of tasks:
    std::mutex m_mutex;
    std::condition_variable cond_var;
    // note that a packaged_task<void> can store a packaged_task<R>:
    std::deque<std::packaged_task<void()>> work;

    // this holds futures representing the worker threads being done:
    std::vector<std::future<void>> finished;

    // queue( lambda ) will enqueue the lambda into the tasks for the threads
    // to use.  A future of the type the lambda returns is given to let you get
    // the result out.
    template<class F, class R = std::result_of_t<F& ()>>
    std::future<R> queue(F&& f) {
        // wrap the function object into a packaged task, splitting
        // execution from the return value:
        std::packaged_task<R()> p(std::forward<F>(f));

        auto r = p.get_future(); // get the return value before we hand off the task
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            work.emplace_back(std::move(p)); // store the task<R()> as a task<void()>
        }
        cond_var.notify_one(); // wake a thread to work on the task

        return r; // return the future result of the task
    }

    // start N threads in the thread pool.
    void start(std::size_t N = 1) {
        for (std::size_t i = 0; i < N; ++i)
        {
            // each thread is a std::async running this->thread_task():
            finished.push_back(
                std::async(
                    std::launch::async,
                    [this] { thread_task(); }
                )
            );
        }
    }
    // abort() cancels all non-started tasks, and tells every working thread
    // stop running, and waits for them to finish up.
    void abort() {
        cancel_pending();
        finish();
    }
    // cancel_pending() merely cancels all non-started tasks:
    void cancel_pending() {
        std::unique_lock<std::mutex> l(m_mutex);
        work.clear();
    }
    // finish enques a "stop the thread" message for every thread, then waits for them:
    void finish() {
        {
            std::unique_lock<std::mutex> l(m_mutex);
            for (auto&& unused : finished) {
                work.push_back({});
            }
        }
        cond_var.notify_all();
        finished.clear();
    }
    ~tasks() {
        finish();
    }
private:
    // the work that a worker thread does:
    void thread_task() {
        while (true) {
            // pop a task off the queue:
            std::packaged_task<void()> lambde_package;

            {
                // usual thread-safe queue code:
                std::unique_lock<std::mutex> lock(m_mutex);
                if (work.empty()) {
                    cond_var.wait(lock, [&] {return !work.empty(); });
                }
                lambde_package = std::move(work.front());
                work.pop_front();
            }
            // if the task is invalid, it means we are asked to abort:
            if (!lambde_package.valid()) return;
            // otherwise, run the task:
            lambde_package();
        }
    }
};
