#include "Task.h"
#include <boost/log/trivial.hpp>
#include <exception>
#include <iostream>

Task::Task(const std::string& name, std::unique_ptr<Worker> worker)
	: name(name), worker(std::move(worker))
{}

void Task::operator()()
{
	try {
		worker->on_start();
		while (worker->do_work());
		worker->on_stop();
	}
	catch (std::exception& ex) {
		BOOST_LOG_TRIVIAL(error) << "Unhandled exception during task [" << name << "] execution! " << ex.what();
	}
	catch (std::string& ex) {
		BOOST_LOG_TRIVIAL(error) << "Unhandled exception during task [" << name << "] execution! " << ex;
	}
	catch (...) {
		BOOST_LOG_TRIVIAL(error) << "Unhandled exception during task [" << name << "] execution!";
	}
}