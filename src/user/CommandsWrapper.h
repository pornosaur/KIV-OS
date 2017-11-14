#pragma once

#include "..\api\api.h"
#include "commands.h"

#include <list>
#include <regex>
#include <vector>
#define SPACE		'\x20'	
#define PIPE		'\x7C'	/* | */
#define LF			'\x0A'	
#define DASH		'\x2D'	/* - */
#define STROKE		'\x2F'  /* / */
#define DOT			'\x2E'	/* . */
#define COLON		'\x3A'	/* : */

#define CMD_ARG			0
#define CMD_INPUT		1

#define WRITE_HANDLE		0
#define READ_HANDLE			1

#define ERR_INCORRECT_CMD	"The syntax of the command is incorrect!"
#define ERR_UNKNOWN_CMD		"The command was not found!"
#define ERR_UNXPECTED		"It was unexpected at this time!"

namespace kiv_os_cmd {

	enum redirect_type
	{
		redirect_to_file, 
		redirect_to_file_append,
		redirect_to_command
	};

	struct redirect_t {
		std::string dest = "";				/* Writes the command output to a file or a device. */
		redirect_type type;
	};

	struct cmd_item_t {
		std::string command = "";
		std::string args_line = "";
		bool is_redirect = false;
		struct redirect_t redirect;
	};

	class CommandsWrapper {
	private:

		static const std::regex r_cmd_line, r_split_pipe, r_command, r_args;
	
		std::string error;
		std::list<struct cmd_item_t> commands;

		bool Parse_Pipe(const std::string& cmd_line);
		bool Parse_Command(struct cmd_item_t& cmd_item);
		bool Parse_Redirect(struct cmd_item_t& cmd_item);
	
	public:
		CommandsWrapper();
		bool Run_Parse(std::string& line);
		void Print_Error();
		void Clear();
		std::vector<kiv_os::THandle> kiv_os_cmd::CommandsWrapper::Run_Commands();

	}; //class CommandsWrapper

} //namespace kiv_os_cmd