// Non_Preemptive_Prriority_Queue.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <hash_map>
#include <queue>
#include <bitset>
#include <iostream>
#include <memory>
#include <thread>
#include <mutex>

using namespace std::chrono;
using namespace std;

const int MAX_PRIORITY = sizeof(unsigned long) * 8;

class ITask
{
public:
	virtual void Run() const = 0;
	virtual unsigned int GetPriority() const = 0;
};

class Task : public ITask
{
public:
	Task(const string& des, unsigned int pri, unsigned int exe) :
		_description(des), _priority(pri), _execute_time(exe)
	{
		_ASSERT(_priority <= MAX_PRIORITY && _priority > 0);
	}

	virtual void Run() const
	{
		cout << _priority << " " << _description << endl;
		this_thread::sleep_for(milliseconds(_execute_time));
	}

	virtual unsigned int GetPriority() const
	{
		return _priority;
	}

	friend bool operator == (const Task& lhs, const Task& rhs)
	{
		return lhs._description == rhs._description;
	}

private:
	string _description;
	unsigned int _priority;
	unsigned int _execute_time;
};

class DefaultTask : public ITask
{
public:
	virtual void Run() const
	{
		this_thread::sleep_for(milliseconds(1));
	}

	virtual unsigned int GetPriority() const
	{
		return 0;
	}
};

shared_ptr<ITask> empty_task = make_shared<DefaultTask>();

class TasksMgr
{
public:
	TasksMgr() : _flags(0)
	{
		_tasks.reserve(MAX_PRIORITY);
		_tasks[0].push(empty_task);
	}

	void push_task(const shared_ptr<ITask>& task)
	{
		_mutex.lock();

		unsigned int index = task->GetPriority();
		_ASSERT(index <= MAX_PRIORITY && index > 0);

		_tasks[index].push(task);
		_flags.set(index);

		_mutex.unlock();
	}

	void emplace_task(const string& des, unsigned int pri, /*double arr, */unsigned int exe)
	{
		_mutex.lock();

		unsigned int index = pri;
		_ASSERT(index <= MAX_PRIORITY && index > 0);

		shared_ptr<ITask> task = make_shared<Task>(des, pri, /*arr,*/ exe);
		_tasks[index].push(task);
		_flags.set(index);
		
		_mutex.unlock();
	}

	shared_ptr<ITask> pop_task()
	{
		_mutex.lock();

		unsigned int index = find_first_bit();
		queue<shared_ptr<ITask>>& tasks = _tasks[index];
		shared_ptr<ITask> tmp = tasks.front();
		
		if (index != 0)
			tasks.pop();

		reset_bit_flag_in_index(index);

		_mutex.unlock();
		return tmp;
	}

private:
	unsigned int find_first_bit()
	{
		unsigned long val = _flags.to_ulong();
		unsigned int index = 0;
		__asm
		{
			push eax
			bsf eax, val //bsf, bsr
			mov index, eax
			pop eax
		}
		return index;
	}

	void reset_bit_flag_in_index(unsigned int index)
	{
		if (_tasks[index].size() == 0)
			_flags.reset(index);
	}

private:
	hash_map<int, queue<shared_ptr<ITask>>> _tasks;
	bitset<MAX_PRIORITY> _flags;
	mutex _mutex;
};

TasksMgr global_tasks;

void scheduler()
{
	while (true)
	{
		shared_ptr<ITask> task = global_tasks.pop_task();
		task->Run();
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	thread th(&scheduler);

	cout << "Sample 1" << endl;
	{
		this_thread::sleep_for(milliseconds(5)); 
		global_tasks.emplace_task("Send data out", 3, 15);
		this_thread::sleep_for(milliseconds(5));
		global_tasks.emplace_task("Update counter", 2, 20);
		this_thread::sleep_for(milliseconds(4));
		global_tasks.emplace_task("Record data", 1, 30);
		this_thread::sleep_for(milliseconds(6));
		global_tasks.emplace_task("Receive data", 4, 10);
		this_thread::sleep_for(milliseconds(1));
		global_tasks.emplace_task("Scan memory", 5, 5);
	}

	this_thread::sleep_for(milliseconds(100));

	cout << "Sample 2" << endl;
	{
		this_thread::sleep_for(milliseconds(5));
		global_tasks.emplace_task("Send package", 3, 30);
		this_thread::sleep_for(milliseconds(5));
		global_tasks.emplace_task("Tax return", 10, 120);
		this_thread::sleep_for(milliseconds(10));
		global_tasks.emplace_task("Make tea", 1, 5);
		this_thread::sleep_for(milliseconds(1));
		global_tasks.emplace_task("Solve RC tasks", 30, 60);
		this_thread::sleep_for(milliseconds(4));
		global_tasks.emplace_task("Feed cat", 1, 5);
		this_thread::sleep_for(milliseconds(35));
		global_tasks.emplace_task("Send email", 20, 15);
	}

	th.join();

	return 0;
}

