#include "Pipe.h"
#include <cassert>

void Pipe::increase_handlers(const uint8_t handle_flag)
{
	if (handle_flag == Handler::fmOpen_Read) {
		readers++;
	}
	else if (handle_flag == Handler::fmOpen_Write) {
		writers++;
	}
}

void Pipe::decrease_handlers(const uint8_t handle_flag)
{
	//TODO: Here should be minimum = 0 -> time to delete pipe!!!
	if (handle_flag == Handler::fmOpen_Read) {
		readers--;
	}
	else if (handle_flag == Handler::fmOpen_Write) {
		writers--;
	}
}

bool Pipe::close_pipe_write() 
{
	writers = 0;
	cv_read.notify_one();

	return !is_pipe_alive();
}

void Pipe::close_pipe_read()
{
	readers = 0;
}

bool Pipe::is_pipe_alive()
{
	return (readers > 0 || writers > 0);
}


bool Pipe::pipe_write(char* buffer, size_t offset, size_t length, size_t& written)
{
	std::unique_lock<std::mutex> lock(buff_m);
	assert(writers);

	//TODO is it necessary to check if the buffer is full?
	while (written_in_buff >= MAX_BUFFER_SIZE) {
		cv_writer.wait(lock);
	}

	size_t tmp_written = 0;
	while (tmp_written < length) {
		size_t to_write = length > MAX_BUFFER_SIZE ? MAX_BUFFER_SIZE : length;
		assert(buffer_pipe);

		memcpy(buffer_pipe, buffer, to_write);
		tmp_written += to_write;
		written_in_buff = tmp_written;
		
		assert(tmp_written <= length);
		assert(readers);
		cv_read.notify_one();	/* TODO: notify one or all? */
		
	}

	written = tmp_written;

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

	memcpy(buffer, buffer_pipe, written_in_buff);
	read = written_in_buff;
	written_in_buff -= read;
	cv_writer.notify_one();  //TODO: discus about it with team
	
	return true;
}