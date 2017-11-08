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
	kiv_os::TProcess_Startup_Info tsi;

	
	char *input = (char *)calloc(MAX_SIZE_BUFFER_IN, sizeof(char));
	kiv_os_cmd::CommandsWrapper cmd_w;

	while (run_shell) {
		kiv_os_rtl::Read_File(kiv_os::stdInput, input, MAX_SIZE_BUFFER_IN, read);
		
		/* Input is not empty; 2 because of \r\n */
		/* TODO: on linux could be less then 2? */
		if (read > 2) {
			if (!cmd_w.Run_Parse(std::string(input))) {
				cmd_w.Print_Error();
				continue;
			}

			tsi.stdin_t = kiv_os::stdInput; //nastaveni std - jiz presmerovanych
			tsi.stdout_t = kiv_os::stdOutput;
			tsi.stderr_t = kiv_os::stdError;
			
			kiv_os::THandle proc_handle;

			std::vector<kiv_os::THandle> proc_handles = cmd_w.Run_Commands(&tsi);
			kiv_os_rtl::Wait_For(proc_handles, 1);
			cmd_w.Clear();
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
