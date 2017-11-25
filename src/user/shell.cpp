#include "shell.h"
#include "string.h"
#include "CommandsWrapper.h"
#include "cd.h"

#include <cstring>
#include <cstdlib>
#include <ctype.h>

static const std::regex line_reg ("([^\\n]+\\n)");

/* Variable for loop in the shell */
int run_system = 1;

size_t __stdcall shell(const kiv_os::TRegisters regs) {
	size_t written, read;
	int run_shell = 1;

	const char* hello = "****KIV-OS****\n";
	kiv_os_rtl::Write_File(kiv_os::stdOutput, hello, strlen(hello), written);
	
	char *input = (char *)malloc(BUFFER_SIZE * sizeof(char));
	if (!input) {
		kiv_os_rtl::print_error("Out of memory.");
		return 0;
	}

	kiv_os_cmd::CommandsWrapper cmd_w;

	while (run_system && run_shell ) {
		print_path(input, 1);

		bool result = kiv_os_rtl::Read_File(kiv_os::stdInput, input, BUFFER_SIZE, read);
		size_t counter = 0;

		if (result && read > 2) {
			std::smatch match;
			std::string tmp_input(input, read);

			while (!tmp_input.empty() && std::regex_search(tmp_input, match, line_reg)) {
				if (!cmd_w.Run_Parse(std::string(match[1].str()))) {
					cmd_w.Print_Error();
					tmp_input = match.suffix().str();
					continue;
				}

				if (!match.suffix().str().empty() || counter > 0) {
					print_path(input, counter, match[1].str());
				}

				std::vector<kiv_os::THandle> proc_handles = cmd_w.Run_Commands();
				kiv_os_rtl::Wait_For(proc_handles, proc_handles.size());
				cmd_w.Clear();

				tmp_input = match.suffix().str();
				counter++;
			}
		}
		
		if (read == 0 || !result) {
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

void print_path(char *input, size_t counter, const std::string &str)
{
	size_t written = 0, read = 0;
	if (counter) {
		kiv_os_rtl::Get_Current_Direcotry(input, BUFFER_SIZE, read);
		kiv_os_rtl::Write_File(kiv_os::stdOutput, "\n", 1, written);
		kiv_os_rtl::Write_File(kiv_os::stdOutput, input, read, written);
		kiv_os_rtl::Write_File(kiv_os::stdOutput, ">", 1, written);
	}

	if (!str.empty()) {
		kiv_os_rtl::Write_File(kiv_os::stdOutput, str.c_str(), str.size(), written);
	}
}
