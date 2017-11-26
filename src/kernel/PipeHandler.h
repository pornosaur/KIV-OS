#include "Handler.h"
#include "Pipe.h"

class PipeHandler : public Handler {

private: 
	/** Appropriate pipe */
	Pipe* pipe;

public:
	/**
	 * Create pipe hadnler with exists pipe.
	 *
	 * @param pipe appropriate pipe
	 * @param flags identifies pipe handler for read or write
	 */
	PipeHandler(Pipe* pipe, uint8_t flags) : Handler(flags), pipe(pipe) {};
	~PipeHandler();

	/**
	 * Read from pipe buffer to output buffer.
	 *
	 * @param buffer output buffer
	 * @param length size of output buffer
	 * @param read read byte
	 * @return error code
	 */
	virtual uint16_t read(char* buffer, size_t length, size_t& read);

	/**
	 * Write to the pipe buffer from input buffer.
	 *
	 * @param buffer input buffer
	 * @param length size of input buffer
	 * @param written written byte to input buffer
	 * @return error code
	 */
	virtual uint16_t write(char* buffer, size_t length, size_t& written);

	/**
	* Method for set position in pipe buffer.
	*
	* @param offset number of bytes to offset from origin
	* @param origin position which reference for offset
	* @param set_size if is set the position will be set as a size
	* @return error code
	*/
	virtual uint16_t fseek(long offset, uint8_t origin, uint8_t set_size);

};