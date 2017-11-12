#pragma once

#include "Handler.h"
#include <condition_variable>
#include <mutex>
#include <cassert>

#define MAX_BUFFER_SIZE		4096	/* maximum size for the pipe buffer */

class Pipe {

private:
	char *buffer_pipe;								/* buffer for data in pipe */
	size_t first, last;								/* pointers where is last and first position in circle buffer */
	size_t readers, writers;						/* flags for reader and writer */
	size_t written_in_buff;							/* size of data in buffer */
	std::condition_variable cv_read, cv_writer;		/* waiting for reader and writer */
	std::mutex buff_m;								/* mutex for reading or writing from buffer */

public:
	bool close_pipe_write();
	bool close_pipe_read();

	bool is_pipe_alive();

	bool pipe_write(char* buffer, size_t offset, size_t length, size_t& written);
	bool pipe_read(char* buffer, size_t offset, size_t length, size_t& read);

	Pipe() : readers(1), writers(1), written_in_buff(0), buffer_pipe(new char[MAX_BUFFER_SIZE]), last(0), first(0) {};
	~Pipe();

}; //class Pipe	