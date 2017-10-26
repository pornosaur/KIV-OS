#pragma once

#include "..\api\api.h"
#include "commands.h"

#include <list>
#include <regex>

#define SPACE		'\x20'
#define PIPE		'\x7C'
#define LF			'\x0A'
#define DASH		'\x2D'
#define STROKE		'\x2F'

#define CMD_ARG			0
#define CMD_INPUT		1


namespace kiv_os_cmd {

	struct arg_t {
		std::string element;
		int type;
	};

	struct redirect_t {
		std::string dest = "";				/* Writes the command output to a file or a device. */
		bool append = false;				/* If was typped >> for append the output to the end of a file. */
	};

	struct cmd_item_t {
		std::string command = "";
		std::string args_line = "";
		std::list<struct arg_t> args_list;
		bool is_redirect = false;
		struct redirect_t redirect;
	};

	class CommandsWrapper {
	private:

		/* Tabulka muze byt max. velikosti uint8_t => 256 - pro nase ucely postacujici */
		struct cmd_function_t {
			const char* name;
			void(CommandsWrapper::*cmd_fun)(const struct cmd_item_t&) = NULL;	 /* Function to handle a command */
			bool(CommandsWrapper::*args_fun)(struct cmd_item_t&) = &CommandsWrapper::Default_Parse_Args;	/* Function to parse args - Default_Parse_Args is default */
		};

		static const std::regex r_cmd_line, r_split_pipe;
	
		std::string error;
		std::list<struct cmd_item_t> commands;

		static cmd_function_t cmd_fcs_list[];

		bool Call_Cmd_Function(cmd_item_t& cmd_item);
		bool Parse_Pipe(std::string& cmd_line);
		bool Default_Parse_Args(struct cmd_item_t& cmd_item);
		bool Echo_Parse_Args(struct cmd_item_t& cmd_item);
		bool Parse_Command(struct cmd_item_t& cmd_item);
		bool Parse_Redirect(struct cmd_item_t& cmd_item);

		void clear();
		void Print_Error();

		/* Commands wrapper */
		void Echo(const struct cmd_item_t&);
	
	public:
		CommandsWrapper();

		bool Run_Parse(std::string& line);

	};
}