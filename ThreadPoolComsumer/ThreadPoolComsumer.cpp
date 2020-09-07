#include <iostream>
#include "ThreadPool.h"
#include "Task.h"

using namespace CustomThreading;

int main()
{
	ThreadPool pool;

	int num = 1;

	{
		auto func = [capt_num = num]() {
			std::cout << "hello " << capt_num << " id " << std::this_thread::get_id() << std::endl;
		};

		auto task = Task::Run(func);
	}

	std::cout << "Hello World! main thread " << std::this_thread::get_id() << std::endl;
	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	}
}
