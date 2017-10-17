#include "Arguments.h"

#include <iostream>

const std::regex kiv_os_cmd::Arguments::r_cmd_line("\\s*(\\w+)\\s*(.*)");
const std::regex kiv_os_cmd::Arguments::r_split_pipe("([^\\|]+)");

kiv_os_cmd::Arguments::Arguments(char* line, size_t size)
{	
	size_line = size;
	cmd_line = std::string(line);

	if (size_line != cmd_line.size()) {
		//TODO: error of input line -> maybe bad convert to std::string
	}

	Parse_Line();

	kiv_os_cmd::Arguments::cmd_item i;
	commands.push_back(i);
}

bool kiv_os_cmd::Arguments::Parse_Line()
{
	std::smatch m_cmd;

	//TODO: make loop if suffix.len of match > 0
	if (std::regex_search(cmd_line, m_cmd, r_split_pipe)) {
		std::string pipe = std::string(m_cmd[1].str());

		//TODO: apply another regex to parse 
	}
	else {
		//TODO: no input was put
	}

	return true;
}

bool kiv_os_cmd::Arguments::Parse_Args(const std::string& args)
{
	std::smatch m_args;

	return true;
}