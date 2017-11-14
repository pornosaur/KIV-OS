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

uint16_t PipeHandler::read(char* buffer, size_t length, size_t& read)
{
	return pipe->pipe_read(buffer, position, length, read);
}



uint16_t PipeHandler::write(char* buffer, size_t length, size_t& written)
{
	return pipe->pipe_write(buffer, position, length, written);
}

uint16_t PipeHandler::fseek(long offset, uint8_t origin)
{
	Handler::position = (size_t)offset;
	// TODO implement
	return kiv_os::erSuccess;
}

