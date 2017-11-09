#include "PipeHandler.h"


PipeHandler::~PipeHandler()
{
	if (get_flags() == kiv_os::fmOpen_Write) {
		if (pipe->close_pipe_write()) {
			delete pipe;
		}
	} 
	else if (get_flags() == kiv_os::fmOpen_Read) {
		pipe->close_pipe_read();

		if (!pipe->is_pipe_alive()) {
			delete pipe;
		}
	}
}

bool PipeHandler::read(char* buffer, size_t offset, size_t length, size_t& read) 
{
	return pipe->pipe_read(buffer, offset, length, read);
}



bool PipeHandler::write(char* buffer, size_t offset, size_t length, size_t& written)
{
	return pipe->pipe_write(buffer, offset, length, written);
}