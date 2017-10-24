#include "Arguments.h"

#include <iostream>

const std::regex kiv_os_cmd::Arguments::r_cmd_line("\\s*(\\w+)([^\\|]+)\\s*");
const std::regex kiv_os_cmd::Arguments::r_split_pipe("\\s*([^\\|]+)");

kiv_os_cmd::Arguments::Arguments(char* line, size_t size)
{
	size_line = size;
	cmd_line = std::string(line);

	if (size_line != cmd_line.size()) {
		//TODO: error of input line -> maybe bad convert to std::string
	}

	if (!Parse_Line()) {
		//TODO: write errot to the shell!!!
	}
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

	while (std::regex_search(check_line, m_cmd, r_split_pipe)) {
		std::string pipe = std::string(m_cmd[1].str());
		check_line = std::string(m_cmd.suffix().str());

		if ((check_line.size() >= 2) && (*check_line.begin() == PIPE)) {
			if ((*(check_line.begin() + 1)) != SPACE || (*(pipe.end() - 1) != SPACE)) {
				error = "Pipe needs left and right side seperated by gaps!";
				return false;
			}
			is_pipe = true;
		}
		else {
			if (is_pipe && !pipe.empty() && (*pipe.begin() == LF)) {
				error = "Right side of pipe is empty!";		
				return false;
			}
			is_pipe = false;
		}

		struct cmd_item_t cmd_item;
		if (!Parse_Args(cmd_item, pipe)) {
			return false;
		}

		commands.push_back(cmd_item);
	}

	return true;
}

bool kiv_os_cmd::Arguments::Parse_Args(struct cmd_item_t& cmd_item, const std::string& args)
{
	std::smatch m_args;
	static std::regex r_args("\"([^\"]+)\"|(\\S+)");

	if (std::regex_match(args, m_args, r_cmd_line)) {
		cmd_item.command = std::string(m_args[1].str());
		std::string str_args = std::string(m_args[2].str());

		if (!Parse_Redirect(cmd_item, str_args)) {
			//TODO: Incorrect format of command
			return false;
		}
		

		while (std::regex_search(str_args, m_args, r_args)) {
			cmd_item.args.push_back(!m_args[1].str().empty() ? m_args[1].str() : m_args[2].str());
			str_args = m_args.suffix();
		}
	}
	else {
		//Command wasnt find
		return false;
	}

	return true;
}

bool kiv_os_cmd::Arguments::Parse_Redirect(struct cmd_item_t& cmd_item, std::string& args)
{
	std::smatch m_redirect;
	static std::regex r_redirect("^([^\\>]*)([\\>]+)([^\\>]*)&");

	if (std::regex_match(args, m_redirect, r_redirect)) {
		if (m_redirect[1].str().empty() || (m_redirect[2].str().size() > 2) || m_redirect[3].str().empty()) {
			return false;
		}

		if (*m_redirect[1].str().end() != SPACE || *m_redirect[3].str().begin() != SPACE) {
			return false;
		}

		//TODO: multiple redirect !!!

		cmd_item.is_redirect = true;
		cmd_item.redirect.append = (m_redirect[2].str().size() == 2);
		cmd_item.redirect.dest = std::string(m_redirect[3].str());
		args = std::string(m_redirect[1].str());
	}
	
	return true;
}