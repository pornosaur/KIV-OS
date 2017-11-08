#include "FileHandler.h"
#include "Pipe.h"

#define IS_EOF(result, read) (!result && read == 0)

class PipeHandler : public FileHandler {

private: 
	Pipe* pipe;

public:
	PipeHandler(Pipe* pipe, uint8_t flags) : FileHandler(flags), pipe(pipe) {};
	~PipeHandler();
	virtual bool read(char* buffer, size_t offset, size_t length, size_t& read);
	virtual bool write(char* buffer, size_t offset, size_t length, size_t& written);

};