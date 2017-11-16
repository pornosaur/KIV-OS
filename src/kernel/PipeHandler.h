#include "Handler.h"
#include "Pipe.h"

class PipeHandler : public Handler {

private: 
	Pipe* pipe;

public:
	PipeHandler(Pipe* pipe, uint8_t flags) : Handler(flags), pipe(pipe) {};
	~PipeHandler();
	virtual uint16_t read(char* buffer, size_t length, size_t& read);
	virtual uint16_t write(char* buffer, size_t length, size_t& written);
	virtual uint16_t fseek(long offset, uint8_t origin, uint8_t set_size);

};