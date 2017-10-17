#pragma once

#include "string.h"

#include <regex>
#include <list>


namespace kiv_os_cmd {

	class Arguments {
	private:

		struct redirect {
			std::string dest;
			bool append;
		};

		struct cmd_item {
			std::string command;
			std::string arg;
			struct redirect redir = {"", 0};
		};

		static const std::regex r_cmd_line, r_split_pipe;

		std::string cmd_line;
		size_t size_line;

		std::list<struct cmd_item> commands;


		bool Parse_Line();
		bool Parse_Args(const std::string& args);

	public:
		Arguments(char* line, size_t size);

		//std::string& Get_Command() { return command; };

	}; //class Arguments

} //namespace kiv_os_cmd