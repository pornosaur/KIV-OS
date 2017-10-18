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
}

bool kiv_os_cmd::Arguments::Parse_Line()
{
	std::smatch m_cmd;
	std::string check_line = cmd_line;
	bool is_pipe = false;

	//TODO: make loop if suffix.len of match > 0
	if (*check_line.begin() == PIPE) {
		//Line starts with PIPE => incorrect
		return false;
	}

	while (!check_line.empty()) {
		if (std::regex_search(check_line, m_cmd, r_split_pipe)) {
			std::string pipe = std::string(m_cmd[1].str());
			check_line = std::string(m_cmd.suffix().str());

			if ((*check_line.begin() == PIPE)) {
				if ((*(check_line.begin() + 1)) != SPACE && (*(pipe.end() - 1) != SPACE)) {
					//TODO: write error -> missed space before and after | (pipe)
					return false;
				}
				is_pipe = true;
			}
			else {
				is_pipe = false;
			}

			//TODO: apply another regex to parse 
		}
		else {
			//TODO: no match with input
		}
	}

	return true;
}

bool kiv_os_cmd::Arguments::Parse_Args(const std::string& args)
{
	std::smatch m_args;

	return true;
}