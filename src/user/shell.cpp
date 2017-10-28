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

	/*tsi.arg = "hello"; //argumenty
	tsi.stdin_t = kiv_os::stdInput; //nastaveni std - jiz presmerovanych
	tsi.stdout_t = kiv_os::stdOutput;
	tsi.stderr_t = kiv_os::stdError;
	kiv_os::THandle proc_handles[10]; //pole s handly procesu
	kiv_os::THandle proc_handle; 
	kiv_os_rtl::Create_Process("echo", &tsi, proc_handle); //vytvoreni procesu
	proc_handles[0] = proc_handle; //pridani handlu do pole s handly
	kiv_os_rtl::Wait_For(proc_handles, 1);*/ //ceka se na dokonceni vsech procesu
	char *input = (char *)calloc(MAX_SIZE_BUFFER_IN, sizeof(char));
	kiv_os_cmd::CommandsWrapper cmd_w;
	while (run_shell) {
		kiv_os_rtl::Read_File(kiv_os::stdInput, input, MAX_SIZE_BUFFER_IN, read);
		
		/* Input is not empty; 2 because of \r\n */
		if (read > 2) {
			if (!cmd_w.Run_Parse(std::string(input))) {
				cmd_w.Print_Error();
			}
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
