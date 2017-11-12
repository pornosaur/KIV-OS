#include "PipeHandler.h"


PipeHandler::~PipeHandler()
{
	if (this->get_flags() == Handler::fmOpen_Write) {
		if (pipe->close_pipe_write()) {
			delete pipe;
		}
	} 
	else if (this->get_flags() == Handler::fmOpen_Read) {
		if (pipe->close_pipe_read()) {
			delete pipe;
		}
	}
}

bool PipeHandler::read(char* buffer, size_t length, size_t& read) 
{
	return pipe->pipe_read(buffer, position, length, read);
}



bool PipeHandler::write(char* buffer, size_t length, size_t& written)
{
	return pipe->pipe_write(buffer, position, length, written);
}

int PipeHandler::fseek(long offset, uint8_t origin)
{
	Handler::position = (size_t)offset;
	// TODO implement
	return 0;
}

