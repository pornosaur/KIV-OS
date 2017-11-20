#include "CommandsWrapper.h"
#include "string.h"
#include "rtl.h"
#include "cd.h"

#include <regex>
#include <memory>
#include <cassert>

const std::regex kiv_os_cmd::CommandsWrapper::r_split_pipe("\\s*([^\\|]+)");
const std::regex kiv_os_cmd::CommandsWrapper::r_command("^([^\\>^\\<^\\s]+)\\s");

kiv_os_cmd::CommandsWrapper::CommandsWrapper()
{
	error = ""; 
}

bool kiv_os_cmd::CommandsWrapper::Run_Parse(std::string& line)
{
	if (!Parse_Pipe(line)) {
		return false;
	}

	assert(error.empty());
	assert(!commands.empty());

	return true;
}

bool kiv_os_cmd::CommandsWrapper::Parse_Pipe(const std::string& cmd_line)
{
	std::smatch m_cmd;
	std::string check_line = cmd_line;

	bool is_pipe = false;

	//TODO check empty string
	if (*check_line.begin() == PIPE) {
		error = ERR_UNXPECTED;
		return false;
	}

	std::string tmp_pipe = "";
	size_t count_char = 0;
	while (std::regex_search(check_line, m_cmd, r_split_pipe)) {
		std::string pipe = tmp_pipe + std::string(m_cmd[1].str());	
		check_line = std::string(m_cmd.suffix().str());

		size_t tmp_count = kiv_os_str::Get_Count_Char(pipe, '"');
		count_char = tmp_count;
		if ((tmp_count & 1) == 1) {
			tmp_pipe += pipe;
			continue;
		}

		if ((check_line.size() >= 2) && (*check_line.begin() == PIPE)) {
			if ((*(check_line.begin() + 1)) != SPACE || (*(pipe.end() - 1) != SPACE)) {
				error = ERR_INCORRECT_CMD;
				return false;
			}
			is_pipe = true;
		}
		else {
			if (is_pipe && !pipe.empty() && (*pipe.begin() == LF)) {
				error = ERR_INCORRECT_CMD;
				return false;
			}
			is_pipe = false;
		}

		struct cmd_item_t cmd_item;
		cmd_item.args_line = pipe;
		
		if (!Parse_Command(cmd_item)) {
			error = ERR_INCORRECT_CMD;
			return false;
		}

		if (!Parse_Redirect(cmd_item)) {
			error = ERR_INCORRECT_CMD;
			return false;
		}

		commands.push_back(cmd_item);
		tmp_pipe.clear();
	}

	if ((count_char & 1) == 1) {
		error = ERR_INCORRECT_CMD;
		return false;
	}

	assert((count_char & 1) != 1);
	return true;
}

bool kiv_os_cmd::CommandsWrapper::Parse_Command(struct cmd_item_t& cmd_item)
{
	std::smatch m_cmd;

	if (std::regex_search(cmd_item.args_line, m_cmd, r_command)) {
		cmd_item.command = std::string(m_cmd[1].str());
		cmd_item.args_line = m_cmd.suffix().str();
		
		if (!cmd_item.args_line.empty() && cmd_item.args_line.back() == SPACE) {
			cmd_item.args_line.pop_back();
		}

		if (!cmd_item.args_line.empty() && cmd_item.args_line.back() != '\n') {
			cmd_item.args_line.append("\r\n");
		}
	}
	else {
		return false;
	}

	return true;
}

bool kiv_os_cmd::CommandsWrapper::Parse_Redirect(struct cmd_item_t& cmd_item)
{
	std::smatch m_redirect;
	static std::regex r_redirect("(\\s*[^\\>^\\<]*)(([\\>\\<]{1,2})\\s?)\\s*([^\\>^\\s^\\<]*)");
	std::string parse_args = cmd_item.args_line;

	struct redirect_t tmp_redir;
	bool redirect = false;

	std::string tmp_redirect = "";
	assert(!cmd_item.command.empty());
	while (std::regex_search(parse_args, m_redirect, r_redirect)) {
		std::string r = tmp_redirect + std::string(m_redirect[1].str());
		

		if ((kiv_os_str::Get_Count_Char(r, '"') & 1) == 1) {
			tmp_redirect += r;
			parse_args = std::string(m_redirect[4].str() + m_redirect.suffix().str());
			continue;
		}

		if (m_redirect[2].str().empty() || m_redirect[4].str().empty()) {
			return false;
		}

		if (*(m_redirect[2].str().end() - 1) != SPACE) {
			return false;
		}

		if (!m_redirect[1].str().empty() && (*(m_redirect[1].str().end() - 1) != SPACE)) {
			return false;
		}

		char redir_first = *m_redirect[3].str().begin();
		char redir_last = *(m_redirect[3].str().end() - 1);

		if (m_redirect[3].str().size() == 2 && redir_first == '>' && redir_last == '>') {
			tmp_redir.type = redirect_type::redirect_to_file_append;
		}
		else if (m_redirect[3].str().size() == 1){
			assert(redir_first == redir_last);
			tmp_redir.type = (redir_first == '>' ? redirect_type::redirect_to_file : redirect_type::redirect_to_command);
		}
		else {
			return false;
		}
	
			tmp_redir.dest = std::string(m_redirect[4].str());
		if (!redirect) {
			cmd_item.args_line = std::string(m_redirect[1].str());
			if (!cmd_item.args_line.empty()) {
				cmd_item.args_line.pop_back(); 
			}
		}
		redirect = true;
		parse_args = m_redirect.suffix();
		tmp_redirect.clear();
	}

	cmd_item.redirect = tmp_redir;
	cmd_item.is_redirect = redirect;

	return true;
}

void kiv_os_cmd::CommandsWrapper::Clear()
{
	error.clear();
	commands.clear();
}

void kiv_os_cmd::CommandsWrapper::Print_Error()
{
	assert(!error.empty());
	if (!error.empty()) {
		error += "\n";
		size_t size = error.size(), written;
		char* c_error = kiv_os_str::copy_string(error);

		kiv_os_rtl::Write_File(kiv_os::stdOutput, c_error, size, written);

		free(c_error);
		c_error = NULL;
	}

	Clear();
}

std::vector<kiv_os::THandle> kiv_os_cmd::CommandsWrapper::Run_Commands()
{
	std::vector<kiv_os::THandle> proc_handles;
	assert(!commands.empty());
	kiv_os::THandle redirect_input_h = kiv_os::erInvalid_Handle;
	kiv_os::THandle redirect_output_h = kiv_os::erInvalid_Handle;
	std::vector<std::vector<kiv_os::THandle>> creation_pipes;
	size_t cmd_counter = 0;
	for (size_t i = 1; i < commands.size(); i++) { //create pipes
		std::vector<kiv_os::THandle> pipe_handles (2);
		kiv_os_rtl::Create_Pipe(&pipe_handles[0]); //0 - write, 1 -read
		creation_pipes.push_back(pipe_handles);
	}
	
	for (const auto &cmd : commands) {
		kiv_os::TProcess_Startup_Info tsi;
		tsi.stdin_t = kiv_os::stdInput; //nastaveni std - jiz presmerovanych
		tsi.stdout_t = kiv_os::stdOutput;
		tsi.stderr_t = kiv_os::stdError;

		//set stdin_t
		if (cmd.is_redirect && cmd.redirect.type == redirect_to_command) { //input redirect
			redirect_input_h = kiv_os_rtl::Create_File(cmd.redirect.dest.c_str(), kiv_os::fmOpen_Always, 0);
			if (redirect_input_h == 0) {
				//TODO get_last_errr
				break;
			}
			tsi.stdin_t = redirect_input_h;
		}
		else {
			if (&cmd == &commands.front()) {
				tsi.stdin_t = kiv_os::stdInput;
			}
			else {
				tsi.stdin_t = creation_pipes[cmd_counter-1][READ_HANDLE];
			}
			
		}

		//set stdout_t
		if (cmd.is_redirect) {
			if (cmd.redirect.type == redirect_to_file) {
				redirect_output_h = kiv_os_rtl::Create_File(cmd.redirect.dest.c_str(), 0, 0);
				if (redirect_output_h == 0) {
					//TODO get_last_errr
					break;
				}
				tsi.stdout_t = redirect_output_h;
				tsi.stderr_t = redirect_output_h;
			}
			if (cmd.redirect.type == redirect_to_file_append) {
				long position = 0;
				redirect_output_h = kiv_os_rtl::Create_File(cmd.redirect.dest.c_str(), kiv_os::fmOpen_Always, 0);
				if (redirect_output_h == 0) {
					//TODO get_last_errr
					break;
				}
				if (!kiv_os_rtl::Set_File_Position(redirect_output_h, position, kiv_os::fsEnd, kiv_os::fsSet_Position)) {
					//TODO get_last_err
					break;
				}
				tsi.stdout_t = redirect_output_h;
				tsi.stderr_t = redirect_output_h;
			}
		}
		else {
			if (&cmd == &commands.back()) {
				tsi.stdout_t = kiv_os::stdOutput;
				tsi.stderr_t = kiv_os::stdError;
			}
			else
			{
				tsi.stdout_t = creation_pipes[cmd_counter][WRITE_HANDLE]; 
				tsi.stderr_t = creation_pipes[cmd_counter][WRITE_HANDLE];
			}
			
		}

		if (cmd.command == "cd") {
			cmd_cd(cmd.args_line);
		}
		else {
			tsi.arg = kiv_os_str::copy_string(cmd.args_line); //argumenty
			kiv_os::THandle proc_handle;
			bool result = kiv_os_rtl::Create_Process(cmd.command.c_str(), tsi, proc_handle); //vytvoreni procesu
			if (result) {
				proc_handles.push_back(proc_handle); //pridani handlu do pole s handly
			}
			else {
				//TODO get_err
				break;
			}
		}
		if (redirect_input_h != kiv_os::erInvalid_Handle) {
			kiv_os_rtl::Close_File(redirect_input_h);
			redirect_input_h = kiv_os::erInvalid_Handle;
		}
		if (redirect_output_h != kiv_os::erInvalid_Handle) {
			kiv_os_rtl::Close_File(redirect_output_h);
			redirect_output_h = kiv_os::erInvalid_Handle;
		}
		cmd_counter++;
		
	}
	/*Close pipe handles*/
	for (std::vector<kiv_os::THandle> pipe_handle : creation_pipes) {
		kiv_os_rtl::Close_File(pipe_handle[WRITE_HANDLE]);
		kiv_os_rtl::Close_File(pipe_handle[READ_HANDLE]);
	}
	return proc_handles;
}
