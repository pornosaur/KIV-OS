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
	
	//TODO if is set offset need it is different size of writing
	size_t new_offset = 0, write = length;
	written = 0;

	while (write > 0) {
		assert(written_in_buff <= MAX_BUFFER_SIZE);
		while (written_in_buff >= MAX_BUFFER_SIZE) {
			if (!readers) {
				close_pipe_write();
				return false;
			}

			cv_writer.wait(lock);
		}
		size_t to_write = (MAX_BUFFER_SIZE - written_in_buff) >= write ? write : MAX_BUFFER_SIZE - written_in_buff;
		size_t write_byte_pos = last + to_write;

		size_t new_write_byte = 0;
		if (write_byte_pos >= MAX_BUFFER_SIZE) {
			new_write_byte = MAX_BUFFER_SIZE - last;
			memcpy(&buffer_pipe[last], &buffer[new_offset], new_write_byte);
			memcpy(&buffer_pipe[0], &buffer[new_offset + new_write_byte], to_write - new_write_byte);
			last = write_byte_pos % MAX_BUFFER_SIZE;
		}
		else {
			memcpy(&buffer_pipe[last], &buffer[new_offset], to_write);
			last += to_write;	//TODO MAYBE + 1 => NEED DEBUG
		}

		assert(new_write_byte <= to_write);
		new_offset += (to_write - new_write_byte);
		written_in_buff += to_write;
		write -= to_write;
		written += to_write;

		cv_read.notify_one();
	}


	//TODO write \0 at the end of writing
	

	return true;
}

bool Pipe::pipe_read(char* buffer, size_t offset, size_t length, size_t& read)
{
	std::unique_lock<std::mutex> lock(buff_m);
	size_t new_length = length - 1;

	while (written_in_buff == 0) {
		if (!writers) {
			read = 0;
			close_pipe_read();
			return false;
		}

		cv_read.wait(lock);
	}

	assert(readers);
	
	size_t new_offset = 0; //TODO it should be deneped on offset from param
	size_t to_read = written_in_buff <= new_length ? written_in_buff : new_length;
	size_t read_byte_pos = first + to_read;

	if (read_byte_pos >= MAX_BUFFER_SIZE) {
		size_t new_read_byte = MAX_BUFFER_SIZE - first;
		memcpy(&buffer[new_offset], &buffer_pipe[first], new_read_byte);
		memcpy(&buffer[new_offset + new_read_byte], &buffer_pipe[0], new_read_byte);
		first = read_byte_pos % MAX_BUFFER_SIZE;
	}
	else {
		memcpy(&buffer[new_offset], &buffer_pipe[first], to_read);
		first += to_read;	//TODO MAYBE + 1 => NEED DEBUG
	}

	buffer[new_offset + to_read] = '\0'; //TODO BE AWARE SIZE OF MALLOC need calculate -1
	read = to_read;
	written_in_buff -= to_read;
	assert(written_in_buff >= 0);
	cv_writer.notify_one();
	
	return true;
}