#include "BinSemaphore.h"

void BinSemaphore::P()
{
	std::unique_lock<std::mutex> guard(m_lock);
	while (val == 0) {
		cond_var.wait(guard);
	}

	val = 0;
}

void BinSemaphore::V() 
{
	std::unique_lock<std::mutex> guard(m_lock);
	if (val == 0) {
		val = 1;
		cond_var.notify_all();
	}
}