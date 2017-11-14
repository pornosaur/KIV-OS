#include "Pipe.h"
#include <cassert>
#include <cstring>

Pipe::~Pipe()
{
	 assert(!is_pipe_alive()); 
	 delete[] buffer_pipe; 

	 buffer_pipe = NULL;

	 assert(!buffer_pipe);
}

bool Pipe::close_pipe_write() 
{
	writers = 0;
	cv_read.notify_one();

	return !is_pipe_alive();
}

bool Pipe::close_pipe_read()
{
	readers = 0;
	cv_writer.notify_one();

	return !is_pipe_alive();
}

bool Pipe::is_pipe_alive()
{
	return (readers > 0 || writers > 0);
}


bool Pipe::pipe_write(char* buffer, size_t offset, size_t length, size_t& written)
{
	std::unique_lock<std::mutex> lock(buff_m);
	assert(writers);

	if (!readers) {
		close_pipe_write();
		return false;
	}
	
	written = 0;
	offset = 0;		//TODO I CAN NOT SET OFFSET FROM USER
	assert(offset <= length);
	length -= offset;

	while (written < length) {
		while (written_in_buff >= MAX_BUFFER_SIZE) {
			if (!readers) {
				close_pipe_write();
				return false;
			}

			cv_writer.wait(lock);
		}

		size_t can_write = (MAX_BUFFER_SIZE - written_in_buff) >= length ? length : MAX_BUFFER_SIZE - written_in_buff;
		can_write = can_write >= (length - written)  ? length - written : can_write;

		if ((last + can_write) >= MAX_BUFFER_SIZE) {
			size_t new_write_byte = MAX_BUFFER_SIZE - last;
			memcpy(&buffer_pipe[last], &buffer[offset], new_write_byte);
			memcpy(&buffer_pipe[0], &buffer[offset + new_write_byte], can_write - new_write_byte);	
		}
		else {
			memcpy(&buffer_pipe[last], &buffer[offset], can_write);
		}

		last = (last + can_write) % MAX_BUFFER_SIZE;
		offset += can_write;
		written += can_write;
	    written_in_buff += can_write;

		cv_read.notify_one();
	}

	assert(written == length);
	
	return true;
}

bool Pipe::pipe_read(char* buffer, size_t offset, size_t length, size_t& read)
{
	std::unique_lock<std::mutex> lock(buff_m);
	offset = 0;		//TODO I CAN NOT SET OFFSET FROM USER
	length -= offset;
	length -= 1;	/* need add \0 to the end of buff */

	while (written_in_buff == 0) {
		if (!writers) {
			read = 0;
			close_pipe_read();
			return false;
		}

		cv_read.wait(lock);
	}

	assert(readers);
	
	size_t can_read = written_in_buff <= length ? written_in_buff : length;

	if ((first + can_read) >= MAX_BUFFER_SIZE) {
		size_t new_read_byte = MAX_BUFFER_SIZE - first;
		memcpy(&buffer[offset], &buffer_pipe[first], new_read_byte);
		memcpy(&buffer[offset + new_read_byte], &buffer_pipe[0], can_read - new_read_byte);		
	}
	else {
		memcpy(&buffer[offset], &buffer_pipe[first], can_read);
	}

	first = (first + can_read) % MAX_BUFFER_SIZE;
	buffer[offset + can_read] = '\0'; 
	read = can_read;
	written_in_buff -= can_read;

	assert(written_in_buff >= 0);
	cv_writer.notify_one();
	
	return true;
}