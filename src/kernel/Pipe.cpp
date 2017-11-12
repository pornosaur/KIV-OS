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

	//TODO Make circle buffer
	while (written_in_buff >= MAX_BUFFER_SIZE) {
		if (!readers) {
			return false;
		}

		cv_writer.wait(lock);
	}

	size_t tmp_written = 0;
	while (tmp_written < length) {
		size_t to_write = length > MAX_BUFFER_SIZE ? MAX_BUFFER_SIZE : length;
		assert(buffer_pipe);
		
		strncpy_s(buffer_pipe, MAX_BUFFER_SIZE, buffer, to_write);	/* TODO discus about memcpy vs strncpy */
		tmp_written += to_write;
		written_in_buff = tmp_written;
		
		assert(tmp_written <= length);
		assert(readers);
		cv_read.notify_one();	
		
	}
	written = tmp_written;

	/*
	size_t offset = 0;
	while (write > 0) {
		while (written >= MAX_BUFF) {
			if (!readers) {
				return false;
			}

			cv_writer.wait(lock);
		}
		size_t to_write = (MAX_BUFF - written) >= write ? write : MAX_BUFF - written;
		size_t pos = last + to_write;

		size_t new_write = 0;
		if (pos >= MAX_BUFF) {
			new_write = MAX_BUFF - (last + 1);
			memcpy(&buff[last + 1], &w[offset], new_write);
			memcpy(&buff[0], &w[offset + new_write], to_write - new_write);
			last = pos % MAX_BUFF;
		}
		else {
			memcpy(&buff[last + 1], &w[offset], pos);
			last += to_write;
		}

		offset += (to_write - new_write);
		written += to_write;
		write -= to_write;
	}
	*/

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