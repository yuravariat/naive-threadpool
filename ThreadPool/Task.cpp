#include "pch.h"
#include "Task.h"

CustomThreading::Task::Task()
	:LambdaWithParams(nullptr)
{
}
CustomThreading::Task::~Task()
{
}
CustomThreading::Task::Task(Task& copyFrom)
{
	//LambdaWithParams = std::packaged_task<void()>(&copyFrom.LambdaWithParams);
}
CustomThreading::Task::Task(Task&& moveFrom)
	:LambdaWithParams(std::move(moveFrom.LambdaWithParams))
{
}
