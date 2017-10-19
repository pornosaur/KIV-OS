#include "Arguments.h"

#include <iostream>

const std::regex kiv_os_cmd::Arguments::r_cmd_line("\\s*(\\w+)\\s*(.*)");
const std::regex kiv_os_cmd::Arguments::r_split_pipe("\\s*([^\\|]+)");

kiv_os_cmd::Arguments::Arguments(char* line, size_t size)
{	
	size_line = size;
	cmd_line = std::string(line);

	if (size_line != cmd_line.size()) {
		//TODO: error of input line -> maybe bad convert to std::string
	}

	Parse_Line();
}

bool kiv_os_cmd::Arguments::Parse_Line()
{
	std::smatch m_cmd;
	std::string check_line = cmd_line;
	bool is_pipe = false;

	if (*check_line.begin() == PIPE) {
		error = "An incorrect command!";
		return false;
	}

	while (!check_line.empty()) {
		if (std::regex_search(check_line, m_cmd, r_split_pipe)) {
			std::string pipe = std::string(m_cmd[1].str());
			check_line = std::string(m_cmd.suffix().str());

			if ((check_line.size() >= 2) && (*check_line.begin() == PIPE)) {
				if ((*(check_line.begin() + 1)) != SPACE || (*(pipe.end() - 1) != SPACE)) {
					error = "Pipe needs left and right side seperated by gaps!";
					return false;
				}
				is_pipe = true;
			} else {
				if (is_pipe && !pipe.empty() && (*pipe.begin() == LF)) {
					error = "Right side of pipe is empty!";		//TODO: put which pipe is incorrect!
					return false;
				}
				is_pipe = false;
			}

			//TODO: apply another regex to parse 
		}
	}

	return true;
}

bool kiv_os_cmd::Arguments::Parse_Args(const std::string& args)
{
	std::smatch m_args;

	return true;
}
