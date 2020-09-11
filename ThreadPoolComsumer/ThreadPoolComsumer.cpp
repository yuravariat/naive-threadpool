#include <iostream>
#include <sstream>
#include "ThreadPool.h"
#include "Task.h"

using namespace CustomThreading;

double someMethod();
void example();
void withErrorsExample();

int main()
{
	withErrorsExample();
}

void example() {
	int num = 1;

	{
		// task that goes out of scope, but still running
		auto func1 = [capt_num = num]() {
			std::stringstream sm;
			sm << "hello int void " << capt_num << " id " << std::this_thread::get_id() << " get out of scope" << std::endl;
			std::cout << sm.str();
		};
		auto task = Task::RunVoid(func1);
	}
	/////////////////////////////////////// 1 /////////////////////////////////////////
	num++;
	auto func = [capt_num = num]() {
		std::stringstream sm;
		std::this_thread::sleep_for(std::chrono::milliseconds(3000));
		sm << "hello task1 int task " << capt_num << " id " << std::this_thread::get_id() << std::endl;
		std::cout << sm.str();
		return 1;
	};
	auto task1 = Task::Run(func);

	////////////////////////////////////// 2 //////////////////////////////////////////
	num++;
	auto task2 = Task::Run([capt_num = num]() {
		std::stringstream sm;
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		sm << "hello string task " << capt_num << " id " << std::this_thread::get_id() << std::endl;
		std::cout << sm.str();
		return std::string("I am string");
		});

	/////////////////////////////////////// 3 /////////////////////////////////////////
	auto task3 = Task::Run(someMethod);


	////////////////////////////////////////////////////////////////////////////////

	std::stringstream sm1;
	sm1 << "Hello World! main thread " << std::this_thread::get_id() << std::endl;
	std::cout << sm1.str();


	Task::WaitAll(task1, task2);

	task3->Wait();

	std::cout << "task1 with id " << task1->GetID() << " returned " << task1->Result << std::endl;
	std::cout << "task2 with id " << task2->GetID() << " returned " << task2->Result << std::endl;
	std::cout << "task3 with id " << task3->GetID() << " returned " << task3->Result << std::endl;
}

void withErrorsExample() {

	auto task1 = Task::Run([]() {
		std::stringstream sm;
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		sm << "hello from task 1 " << " id " << std::this_thread::get_id() << std::endl;
		std::cout << sm.str();
		throw std::exception("exception from task1");
		return std::string("I am string");
		});

	auto task2 = Task::Run([]() {
		std::stringstream sm;
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		sm << "hello from task 1 " << " id " << std::this_thread::get_id() << std::endl;
		std::cout << sm.str();
		return 1;
		});

	Task::WaitAll(task1, task2);

	if (task1->GetStatus() == CustomThreading::TaskStatus::Faulted) {
		std::cout << "task1 thorwn \"" << task1->exception.what() << "\"" << std::endl;
	}
	else {
		std::cout << "task1 with id " << task1->GetID() << " returned " << task1->Result << std::endl;
	}

	if (task2->GetStatus() == CustomThreading::TaskStatus::Faulted) {
		std::cout << "task2 thorwn \"" << task2->exception.what() << "\"" << std::endl;
	}
	else {
		std::cout << "task2 with id " << task2->GetID() << " returned " << task2->Result << std::endl;
	}
}

double someMethod() {
	std::stringstream sm;
	sm << "hello double task " << " id " << std::this_thread::get_id() << std::endl;
	std::cout << sm.str();
	return 1.5;
}
