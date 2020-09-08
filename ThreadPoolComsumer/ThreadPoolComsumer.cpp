#include <iostream>
#include <sstream>
#include "ThreadPool.h"
#include "Task.h"

using namespace CustomThreading;

int main()
{
	ThreadPool pool;

	int num = 1;

	{
		auto func = [capt_num = num]() {
			std::stringstream sm;
			sm << "hello int task " << capt_num << " id " << std::this_thread::get_id() << " get out of scope" << std::endl;
			std::cout << sm.str();
			return 1;
		};
		auto task = Task::Run(func);
	}

	auto func = [capt_num = num]() {
		std::stringstream sm;
		sm << "hello int task " << capt_num << " id " << std::this_thread::get_id() << std::endl;
		std::cout << sm.str();
		return 1;
	};
	auto task1 = Task::Run(func);
	auto task2 = Task::Run([capt_num = num]() {
		std::stringstream sm;
		sm << "hello string task " << capt_num << " id " << std::this_thread::get_id() << std::endl;
		std::cout << sm.str();
		return std::string("I am string");
		});

	std::stringstream sm1;
	sm1 << "Hello World! main thread " << std::this_thread::get_id() << std::endl;
	std::cout << sm1.str();
	

	// task.Wait still not implemeted, so we are waiting.
	std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	if (task1->GetStatus() == CustomThreading::TaskStatus::RanToCompletion) {
		std::cout << "task1 returned " << task1->Result << std::endl;
	}
	if (task2->GetStatus() == CustomThreading::TaskStatus::RanToCompletion) {
		std::cout << "task2 returned " << task2->Result << std::endl;
	}
}
