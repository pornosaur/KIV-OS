#include "PipeHandler.h"


PipeHandler::~PipeHandler()
{
	if (get_flags() == kiv_os::fmOpen_Write) {
		//TODO: maybe close should be returned bool 
		pipe->close_pipe_write();
	}
}

bool PipeHandler::read(char* buffer, size_t offset, size_t length, size_t& read) 
{
	//TODO: fixed all method to returned bool and read in param!!s
	//bool read_result = pipe->pipe_read(buffer, offset, length, read);

	if (IS_EOF(read, 1)) {
		//TODO: EOF was sent

		return false;
	}

	return true;
}



bool PipeHandler::write(char* buffer, size_t offset, size_t length, size_t& written)
{

	return 0;
}