#pragma once
#include "Worker.h"
#include <memory>
#include <utility>
#include <string>

class Task
{
	const std::string name;
	std::unique_ptr<Worker> worker;

public:
	Task(const std::string& name, std::unique_ptr<Worker> worker);
	void operator()();
};