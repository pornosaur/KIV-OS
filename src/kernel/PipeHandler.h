#include "FileHandler.h"
#include "Pipe.h"


class PipeHandler : public FileHandler {

private:
	Pipe *pipe;

public:
	virtual size_t read(char* buffer, size_t offset, size_t length);
	virtual size_t write(char* buffer, size_t offset, size_t length);

};