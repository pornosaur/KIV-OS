#include "Pipe.h"
#include <cassert>
#include <cstring>

Pipe::~Pipe()
{
	 assert(!is_pipe_alive()); 
	 delete[] buffer_pipe; 

	 buffer_pipe = NULL;
	 first = NULL;
	 last = NULL;

	 assert(!buffer_pipe && !first && !last);
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
	
	//TODO if is set offset need it is different size of writing
	size_t new_offset = 0, write = length;
	while (write > 0) {
		while (written_in_buff >= MAX_BUFFER_SIZE) {
			if (!readers) {
				close_pipe_write();
				return false;
			}

			cv_writer.wait(lock);
		}
		size_t to_write = (MAX_BUFFER_SIZE - written_in_buff) >= write ? write : MAX_BUFFER_SIZE - written_in_buff;
		size_t write_byte = last + to_write;

		size_t new_write_byte = 0;
		if (write_byte >= MAX_BUFFER_SIZE) {
			new_write_byte = MAX_BUFFER_SIZE - last;
			memcpy(&buffer_pipe[last], &buffer[new_offset], new_write_byte);
			memcpy(&buffer_pipe[0], &buffer[new_offset + new_write_byte], to_write - new_write_byte);
			last = write_byte % MAX_BUFFER_SIZE;
		}
		else {
			memcpy(&buffer_pipe[last], &buffer[new_offset], write_byte);
			last += to_write;	//TODO MAYBE + 1 => NEED DEBUG
		}

		assert(new_write_byte <= to_write);
		new_offset += (to_write - new_write_byte);
		written_in_buff += to_write;
		write -= to_write;

		cv_read.notify_one();
	}

	//TODO write \0 at the end of writing
	

	return true;
}

bool Pipe::pipe_read(char* buffer, size_t offset, size_t length, size_t& read)
{
	std::unique_lock<std::mutex> lock(buff_m);

	while (written_in_buff == 0) {
		if (!writers) {
			read = 0;
			close_pipe_read();
			return false;
		}

		cv_read.wait(lock);
	}

	assert(readers);
	strncpy_s(buffer, length, buffer_pipe, written_in_buff);		/* TODO discus about memcpy vs strncpy */
	read = written_in_buff;
	written_in_buff -= read;
	cv_writer.notify_one();  //TODO: discus about it with team
	
	return true;
}