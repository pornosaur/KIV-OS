#include "Arguments.h"

const std::regex r_cmd_line("\\s*(\\w+)\\s*(.*)");

kiv_os_cmd::Arguments::Arguments(char* line, size_t size)
{
	std::cmatch m_cmd;
	cmd_line = std::string(line);

	/*if (std::regex_search(cmd_line, match, regex_cmd)) {

	}*/
}