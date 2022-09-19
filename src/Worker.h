#pragma once

struct Worker
{
	virtual void on_start() = 0;
	virtual bool do_work() = 0;
	virtual void on_stop() = 0;
	virtual ~Worker(){}
};