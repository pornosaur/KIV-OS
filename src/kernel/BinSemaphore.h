#pragma once

#include <mutex>
#include <condition_variable>

class BinSemaphore {
private:
	/** Value of semaphore */
	int val;
	/** Mutex for critical section */
	std::mutex m_lock;
	/** List of blocked thread until notified to resume */
	std::condition_variable cond_var;

public:

	/**
	 * Create binary semaphore which initaliazed value to 1. It is for purpose of lock the kernel.
	 */
	BinSemaphore() : val(1) {};

	/**
	 * Set value to 0 or block thread.
	 */
	void P();

	/**
	 * Set value to 1 if it is 0 and notify thread waiting on condition variable.
	 */
	void V();

};