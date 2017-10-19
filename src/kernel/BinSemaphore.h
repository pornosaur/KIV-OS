#pragma once

#include <mutex>
#include <condition_variable>

class BinSemaphore {
private:
	int val;
	std::mutex m_lock;
	std::condition_variable cond_var;

public:
	BinSemaphore() : val(1) {};

	void P();
	void V();

};