#include "CommandsWrapper.h"
#include "string.h"
#include "rtl.h"

#include <regex>
#include <memory>
#include <cassert>

 kiv_os_cmd::CommandsWrapper::cmd_function_t kiv_os_cmd::CommandsWrapper::cmd_fcs_list[] = {
	/*{ "type", type },
	{ "md",  md },
	{ "rd",  rd },*/
	{ "echo",  &CommandsWrapper::Echo, &CommandsWrapper::Echo_Parse_Args },
	{ "echo.", &CommandsWrapper::Echo, &CommandsWrapper::Echo_Parse_Args },
	{ "echo:", &CommandsWrapper::Echo, &CommandsWrapper::Echo_Parse_Args }
	/*{ "wc", wc },
	{ "sort", sort },
	{ "rgen", rgen },
	{ "freq", freq },
	{ "ps", ps },
	{ "shutdown", shutdown }*/
};

const std::regex kiv_os_cmd::CommandsWrapper::r_cmd_line("\\s*(\\w+)([^\\|]+)\\s*");
const std::regex kiv_os_cmd::CommandsWrapper::r_split_pipe("\\s*([^\\|]+)");
const std::regex kiv_os_cmd::CommandsWrapper::r_command("(\\S+)");
const std::regex kiv_os_cmd::CommandsWrapper::r_args("\"([^\"]+)\"|(([-\\/]*)\\S+)");


kiv_os_cmd::CommandsWrapper::CommandsWrapper()
{
	error = ""; 
}

bool kiv_os_cmd::CommandsWrapper::Run_Parse(std::string& line)
{
	if (!Parse_Pipe(line)) {
		return false;
	}

	for (auto cmd_item : commands)
	{
		if (!Call_Cmd_Function(cmd_item)) {
			return false;
		}
	}

	assert(error.empty());
	assert(!commands.empty());
	clear();

	return true;
}


bool kiv_os_cmd::CommandsWrapper::Call_Cmd_Function(cmd_item_t& cmd_item)
{
	uint8_t i, size = sizeof(cmd_fcs_list) / sizeof(cmd_fcs_list[0]);
	kiv_os_str::string_to_lower(cmd_item.command);

	for (i = 0; i < size; i++) {
		if (cmd_fcs_list[i].cmd_fun && (cmd_item.command.compare(cmd_fcs_list[i].name) == 0)) {
			(this->*cmd_fcs_list[i].args_fun)(cmd_item);	/* Prepare arguments */
			(this->*cmd_fcs_list[i].cmd_fun)(cmd_item);		/* Call method of command to check all arguments */ 
			return true;
		}
	}

	error = "The command was not found!";
	return false;
}

bool kiv_os_cmd::CommandsWrapper::Parse_Pipe(std::string& cmd_line)
{
	std::smatch m_cmd;
	std::string check_line = cmd_line;

	bool is_pipe = false;

	if (*check_line.begin() == PIPE) {
		error = "An incorrect command!";
		return false;
	}

	std::string tmp_pipe = "";
	while (std::regex_search(check_line, m_cmd, r_split_pipe)) {
		std::string pipe = tmp_pipe + std::string(m_cmd[1].str());
		check_line = std::string(m_cmd.suffix().str());

		if ((kiv_os_str::Get_Count_Char(pipe, '"') & 1) == 1) {
			tmp_pipe += pipe;
			continue;
		}

		if ((check_line.size() >= 2) && (*check_line.begin() == PIPE)) {
			if ((*(check_line.begin() + 1)) != SPACE || (*(pipe.end() - 1) != SPACE)) {
				error = "A pipe needs left and right side seperated by gap!";
				return false;
			}
			is_pipe = true;
		}
		else {
			if (is_pipe && !pipe.empty() && (*pipe.begin() == LF)) {
				error = "The right side of a pipe is empty!";
				return false;
			}
			is_pipe = false;
		}

		struct cmd_item_t cmd_item;
		cmd_item.args_line = pipe;
		
		if (!Parse_Command(cmd_item)) {
			return false;
		}

		if (!Parse_Redirect(cmd_item)) {
			error = "Incorrect structure of redirect!";
			return false;
		}

		commands.push_back(cmd_item);
		tmp_pipe.clear();
	}

	return true;
}

bool kiv_os_cmd::CommandsWrapper::Parse_Command(struct cmd_item_t& cmd_item)
{
	std::smatch m_cmd;

	if (std::regex_search(cmd_item.args_line, m_cmd, r_command)) {
		cmd_item.command = std::string(m_cmd[1].str());
		cmd_item.args_line = m_cmd.suffix().str();
	}
	else {
		error = "The command missing!";
		return false;
	}

	return true;
}

bool kiv_os_cmd::CommandsWrapper::Parse_Redirect(struct cmd_item_t& cmd_item)
{
	std::smatch m_redirect;
	static std::regex r_redirect("\\s*([^\\>]*)(\\s([\\>]{1,2})\\s)\\s*([^\\>^\\s]*)");

	if (std::regex_search(cmd_item.args_line, m_redirect, r_redirect)) {
		if (m_redirect[1].str().empty() || m_redirect[2].str().empty() || m_redirect[4].str().empty()) {
			return false;
		}

		if ((*(m_redirect[2].str().end() - 1) != SPACE) || (*m_redirect[2].str().begin() != SPACE)) {
			return false;
		}

		//TODO: multiple redirect !!!

		cmd_item.is_redirect = true;
		cmd_item.redirect.append = (m_redirect[3].str().size() == 2);
		cmd_item.redirect.dest = std::string(m_redirect[4].str());
		cmd_item.args_line = std::string(m_redirect[1].str());
	}

	return true;
}

bool kiv_os_cmd::CommandsWrapper::Default_Parse_Args(struct cmd_item_t& cmd_item)
{
	std::smatch m_args;
	
	while (std::regex_search(cmd_item.args_line, m_args, r_args)) {
		cmd_item.args_list.push_back(
			{
				!m_args[1].str().empty() ? m_args[1].str() : m_args[2].str(),
				m_args[3].str().empty() ? CMD_INPUT : CMD_ARG
			}
		);

		cmd_item.args_line = m_args.suffix();
	}

	return true;
}

bool kiv_os_cmd::CommandsWrapper::Echo_Parse_Args(struct cmd_item_t& cmd_item)
{
	std::smatch m_args;
	static std::regex r_echo("\\s(.*)");

	if ((*(cmd_item.command.end() - 1) == DOT) || (*(cmd_item.command.end() - 1) == COLON)) {
		cmd_item.args_list.push_back(
			{
				" ",
				CMD_INPUT
			}
		);
	}

	while (std::regex_search(cmd_item.args_line, m_args, r_echo)) {
		cmd_item.args_list.push_back(
			{
				!m_args[1].str().empty() ? m_args[1].str() : m_args[2].str(),
				CMD_INPUT
			}
		);

		cmd_item.args_line = m_args.suffix();
	}

	return true;
}

void kiv_os_cmd::CommandsWrapper::clear()
{
	error.clear();
	commands.clear();
}

void kiv_os_cmd::CommandsWrapper::Print_Error()
{
	if (!error.empty()) {
		error += "\n";
		size_t size = error.size(), written;
		char* c_error = kiv_os_str::copy_string(error);

		//TODO: change stdOutput to stdError
		kiv_os_rtl::Write_File(kiv_os::stdOutput, c_error, size, written);

		free(c_error);
		c_error = NULL;
	}

	clear();
}

void kiv_os_cmd::CommandsWrapper::Echo(const struct cmd_item_t& cmd_item)
{
	std::string result_echo = "";
	for (auto arg : cmd_item.args_list) {
		result_echo += arg.element;
	}

	if (!result_echo.empty()) {
		result_echo += '\n';
	}

	kiv_os::TRegisters regs;
	char* t = kiv_os_str::copy_string(result_echo);
	regs.rdx.r = reinterpret_cast<decltype(regs.rdx.r)>(t);

	//size_t size = echo(regs);
}