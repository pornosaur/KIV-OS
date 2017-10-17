#pragma once

#include "..\api\api.h"
#include "commands.h"

#include "Arguments.h"

namespace kiv_os_cmd {

	/* Tabulka muze byt max. velikosti uint8_t => 256 - pro nase ucely postacujici */
	struct cmd_function_t {
		const char* name;
		size_t(__stdcall *cmd_fun)(const kiv_os::TRegisters&) = NULL;
	};

	class CommandsWrapper {
	private:
	
		static const cmd_function_t cmd_fcs_list[];

		int call_cmd_function(char* cmd_name, const char* params);

	public:
		CommandsWrapper(kiv_os_cmd::Arguments& args);

	};
}