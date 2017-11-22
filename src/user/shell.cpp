#include "shell.h"
#include "string.h"
#include "CommandsWrapper.h"
#include "cd.h"

#include <cstring>
#include <cstdlib>
#include <regex>
#include <ctype.h>


/* Variable for loop in the shell */
int run_system = 1;

size_t __stdcall shell(const kiv_os::TRegisters &regs) {
	size_t written, read;
	int run_shell = 1;

	const char* hello = "****KIV-OS****\n";
	kiv_os_rtl::Write_File(kiv_os::stdOutput, hello, strlen(hello), written);
	
	char *input = (char *)calloc(MAX_SIZE_BUFFER_IN, sizeof(char));
	kiv_os_cmd::CommandsWrapper cmd_w;

	while (run_system && run_shell ) {
		kiv_os_rtl::Get_Current_Direcotry(input, BUFFER_SIZE, read);
		kiv_os_rtl::Write_File(kiv_os::stdOutput, input, read, written);
		kiv_os_rtl::Write_File(kiv_os::stdOutput, ">", 1, written);

		bool result = kiv_os_rtl::Read_File(kiv_os::stdInput, input, MAX_SIZE_BUFFER_IN, read);

		if (result && read > 2) {
			if (!cmd_w.Run_Parse(std::string(input, read))) {
				cmd_w.Print_Error();
				continue;
			}			

			std::vector<kiv_os::THandle> proc_handles = cmd_w.Run_Commands();
			kiv_os_rtl::Wait_For(proc_handles, proc_handles.size());
			cmd_w.Clear();
		}
		
		if (result && read == 0) {
			run_shell = 0;
		}
	}

	free(input);
	input = nullptr;
	
	return 0;
}

void __stdcall system_stop()
{
	run_system = 0;
}
