#pragma once

#include "FileHandler.h"
#include <condition_variable>
#include <mutex>
#include <cassert>

#define MAX_BUFFER_SIZE		4096	/* maximum size for pipe buffer */

class Pipe {

private:
	char* buffer_pipe;								/* buffer for data in pipe */
	size_t readers, writers;						/* flags for reader and writer */
	size_t written_in_buff;							/* size of data in buffer */
	std::condition_variable cv_read, cv_writer;		/* waiting for reader and writer */
	std::mutex buff_m;								/* mutex for reading or writing from buffer */

public:
	void increase_handlers(const uint8_t handle_flag);
	void decrease_handlers(const uint8_t handle_flag);

	bool close_pipe_write();
	void close_pipe_read();

	bool is_pipe_alive();

	bool pipe_write(char* buffer, size_t offset, size_t length, size_t& written);
	bool pipe_read(char* buffer, size_t offset, size_t length, size_t& read);

	Pipe() : readers(1), writers(1), written_in_buff(0), buffer_pipe(new char[MAX_BUFFER_SIZE]) {};
	~Pipe() { assert(!is_pipe_alive()); delete[] buffer_pipe; };

}; //class Pipe	