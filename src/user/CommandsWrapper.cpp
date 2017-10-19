#include "CommandsWrapper.h"
#include "string.h"

const kiv_os_cmd::cmd_function_t kiv_os_cmd::CommandsWrapper::cmd_fcs_list[] = {
	{ "type", type },
	{ "md",  md },
	{ "rd",  rd },
	{ "echo",  echo },
	{ "echo.", echo },	//TODO: promyslet, jestli neudelat <tuple>?? zalezi, jestli existuje vice variant zapisu i u ostatnich
	{ "wc", wc },
	{ "sort", sort },
	{ "rgen", rgen },
	{ "freq", freq },
	{ "ps", ps },
	{ "shutdown", shutdown }
};


kiv_os_cmd::CommandsWrapper::CommandsWrapper(kiv_os_cmd::Arguments& args)
{
		//init
}

int kiv_os_cmd::CommandsWrapper::call_cmd_function(char* cmd_name, const char* params)
{
	uint8_t i, size = sizeof(cmd_fcs_list) / sizeof(cmd_fcs_list[0]);
	kiv_os::TRegisters regs;
	regs.rdx.r = reinterpret_cast<decltype(regs.rdx.r)>(params);
	kiv_os_str::string_to_lower(cmd_name, strlen(cmd_name));

	for (i = 0; i < size; i++) {
		if (cmd_fcs_list[i].cmd_fun && (strcmp(cmd_name, cmd_fcs_list[i].name) == 0)) {
			cmd_fcs_list[i].cmd_fun(regs);
			return 0;
		}
	}

	return -1;
}