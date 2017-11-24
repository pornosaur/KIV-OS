#pragma once

#include "Handler.h"

#include <condition_variable>
#include <mutex>
#include <cassert>

#define MAX_BUFFER_SIZE		4096	/* cyclic buffer size */

class Pipe {

private:
	char *buffer_pipe;								/* buffer for data in pipe */
	size_t first, last;								/* last and first is position in cyclic buffer */
	size_t readers, writers;						/* flags for reader and writer */
	size_t written_in_buff;							/* size of data in buffer */
	std::condition_variable cv_read, cv_writer;		/* waiting for reader and writer */
	std::mutex buff_m;								/* mutex for reading or writing from buffer */

public:

	/**
	 * Close the pipe for write, writer is set to 0. After that the pipe will generate EOF.
	 *
	 * @return true if the pipe is empty; reader = 0 and writer = 0.
	 */
	bool close_pipe_write();

	/**
	 * Close the pipe for read, reader is to 0.
	 *
	 * @return true if the pipe is empty; reader = 0 and writer = 0;
	 */
	bool close_pipe_read();

	/**
	 * Check if there is set reader and writer in the pipe.
	 *
	 * @return true if reader >= 1 and writer >= 1.
	 */
	bool is_pipe_alive();
	 
	/**
	 * Write content of input buffer to the pipe buffer.
	 *
	 * @param buffer input buffer 
	 * @param offset position in input buffer
	 * @param length size of input buffer
	 * @param written written byte to the pipe buffer
	 * @return error code
	 */
	uint16_t pipe_write(char* buffer, size_t offset, size_t length, size_t& written);

	/**
	 * Read content of the pipe buffer to output buffer.
	 * 
	 * @param buffer output buffer
	 * @param offset position in the pipe buffer
	 * @param length size of output buffer
	 * @param read read byte from the pipe buffer
	 * @return error code
	 */
	uint16_t pipe_read(char* buffer, size_t offset, size_t length, size_t& read);

	/**
	 * Create new pipe with default value of variables. Reader and writer are set to 1, init buffer to MAX_BUFFER_SIZE,
	 * last and first are set to 0.
	 */
	Pipe() : readers(1), writers(1), written_in_buff(0), buffer_pipe(new char[MAX_BUFFER_SIZE]), last(0), first(0) {};

	/**
	 * Free memory of buffer.
	 */
	~Pipe();

}; //class Pipe	