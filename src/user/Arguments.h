#pragma once

#include "string.h"
#include <regex>
#include <list>

#define SPACE		'\x20'
#define PIPE		'\x7C'
#define LF			'\x0A'

namespace kiv_os_cmd {

	struct redirect_t {
		std::string dest = "";				/* Writes the command output to a file or a device. */
		bool append = false;				/* If was typped >> for append the output to the end of a file. */
	};

	struct cmd_item_t {
		std::string command = "";
		std::list<std::string> args;
		bool is_redirect = false;
		struct redirect_t redirect;
	};

	class Arguments {
	private:

		static const std::regex r_cmd_line, r_split_pipe;

		std::string cmd_line, error;
		size_t size_line;

		std::list<struct cmd_item_t> commands;


		bool Parse_Line();
		bool Parse_Args(struct cmd_item_t& cmd, const std::string& args);
		bool Parse_Redirect(struct cmd_item_t& cmd_item, std::string& args);

	public:
		Arguments(char* line, size_t size);


	}; //class Arguments

} //namespace kiv_os_cmd