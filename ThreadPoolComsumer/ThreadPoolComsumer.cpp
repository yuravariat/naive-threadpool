#include <iostream>
#include "ThreadPool.h"
#include "Task.h"

int main()
{
	CustomThreading::ThreadPool pool;

	int num = 1;

	auto func = [capt_num = num]() {
		std::cout << "hello " << capt_num << " id " << std::this_thread::get_id() << std::endl;
	};

	CustomThreading::Task task(func);

	pool.QuqueTask(&task);

	std::cout << "Hello World! main thread " << std::this_thread::get_id() << std::endl;
	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(20000));
	}
}
