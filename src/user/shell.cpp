#include "shell.h"


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

		if (std::regex_search(input, match, regex_cmd) && (match.size() == REGEX_DEF_GROUP)) {
			
			char *params = copy_string(match[2].str());
			if (!match[2].str().empty() && std::regex_search(params, m_redirect, regex_redirect)) {
				const char* msg = "\tRedirect found\n";
				kiv_os_rtl::Write_File(stdinn, msg, strlen(msg), error_write);
				char *new_params = copy_string(m_redirect[1].str());
				free(params);
				params = new_params;
			}

			char *cmd_name = copy_string(match[1].str());
			if (call_cmd_function(cmd_name, params) == ERROR_RESULT) {
				kiv_os_rtl::Write_File(stdinn, error_dialog, strlen(error_dialog), error_write);
			}

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

size_t __stdcall echo(const kiv_os::TRegisters &regs)
{
	const char* params = reinterpret_cast<char*>(regs.rdx.r);
	size_t writen;

	if (strlen(params) == 0) {
		kiv_os_rtl::Write_File(stdinn, error_dialog, strlen(error_dialog), error_write);
		return ERROR_RESULT;
	}

	kiv_os_rtl::Write_File(stdinn, params, strlen(params), writen);

	return CORRECT_RESULT;
}

int8_t call_cmd_function(char* cmd_name, const char* params)
{
	uint8_t i, size = sizeof(Cmd_Function) / sizeof(Cmd_Function[0]);
	kiv_os::TRegisters regs;
	regs.rdx.r = reinterpret_cast<decltype(regs.rdx.r)>(params);
	string_to_lower(cmd_name);

	for (i = 0; i < size; i++) {
		if (Cmd_Function[i].cmd_fun && (strcmp(cmd_name, Cmd_Function[i].name) == 0)) {
			Cmd_Function[i].cmd_fun(regs);
			return 0;
		}
	}

	return -1;
}

void string_to_lower(char* str)
{
	size_t len = strlen(str);
	for (size_t i = 0; i < len; i++) {
		str[i] = tolower(str[i]);
	}
}

char *copy_string(const std::string &str)
{
	size_t len = str.size();
	char *new_str = (char*)malloc(sizeof(char) * (len + 1u));
	strcpy_s(new_str, len + 1u, str.c_str());
	new_str[len] = '\0';
	
	return new_str;
}