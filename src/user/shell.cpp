#include "shell.h"
#include "string.h"
#include "Arguments.h"

#include <cstring>
#include <cstdlib>
#include <regex>
#include <ctype.h>


size_t __stdcall shell(const kiv_os::TRegisters &regs) {
	/* First group => command; Second group => parametrs */
	static const std::regex regex_cmd("\\s*(\\w+)\\s*(.*)");
	static const std::regex regex_redirect("(.*)\\s>{1,2}\\s([\\w\\._]+)");
	static std::cmatch match, m_redirect;

	size_t written, read, run = 1;

	const char* hello = "****KIV-OS****\n";
	kiv_os_rtl::Write_File(stdinn, hello, strlen(hello), written);

	char *input = (char *)calloc(MAX_SIZE_BUFFER_IN, sizeof(char));
	while (run) {
		kiv_os_rtl::Read_File(stdio, input, MAX_SIZE_BUFFER_IN, read);
		kiv_os_cmd::Arguments a(input, 50);

		if (std::regex_search(input, match, regex_cmd) && (match.size() == REGEX_DEF_GROUP)) {
			
			char *params = kiv_os_str::copy_string(match[2].str());
			if (!match[2].str().empty() && std::regex_search(params, m_redirect, regex_redirect)) {
				const char* msg = "\tRedirect found\n";
				kiv_os_rtl::Write_File(stdinn, msg, strlen(msg), error_write);
				char *new_params = kiv_os_str::copy_string(m_redirect[1].str());
				free(params);
				params = new_params;
			}

			char *cmd_name = kiv_os_str::copy_string(match[1].str());
			/*if (call_cmd_function(cmd_name, params) == ERROR_RESULT) {
				kiv_os_rtl::Write_File(stdinn, error_dialog, strlen(error_dialog), error_write);
			}*/

			free(cmd_name);
			free(params);
		}
		else {
			kiv_os_rtl::Write_File(stdinn, error_dialog, strlen(error_dialog), error_write);
		}

		input = (char *)calloc(MAX_SIZE_BUFFER_IN, sizeof(char));
	}
			
	kiv_os_rtl::Close_File(stdinn);
	free(input);
	
	return 0;
}

void __stdcall shell_stop()
{
	//TODO: implement shell stop 
}


