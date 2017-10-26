#include "shell.h"
#include "string.h"
#include "CommandsWrapper.h"

#include <cstring>
#include <cstdlib>
#include <regex>
#include <ctype.h>


size_t __stdcall shell(const kiv_os::TRegisters &regs) {
	size_t written, read;

	const char* hello = "****KIV-OS****\n";
	kiv_os_rtl::Write_File(kiv_os::stdOutput, hello, strlen(hello), written);

	char *input = (char *)calloc(MAX_SIZE_BUFFER_IN, sizeof(char));
	kiv_os_cmd::CommandsWrapper cmd_w;
	while (run_shell) {
		kiv_os_rtl::Read_File(kiv_os::stdInput, input, MAX_SIZE_BUFFER_IN, read);

		/* Input is not empty; 2 because of \r\n */
		if (read > 2) {
			cmd_w.Run_Parse(std::string(input));
		}

		input = (char *)calloc(MAX_SIZE_BUFFER_IN, sizeof(char));
	}

	free(input);
	input = NULL;

	return 0;
}

void __stdcall shell_stop()
{
	run_shell = 0;
	//TODO: implement also shell stop in the Kernel
}
