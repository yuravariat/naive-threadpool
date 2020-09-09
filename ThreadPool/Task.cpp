#include "pch.h"
#include "Task.h"

std::atomic<unsigned long> CustomThreading::Task::_idCounter;

CustomThreading::Task::_init CustomThreading::Task::_initializer;
