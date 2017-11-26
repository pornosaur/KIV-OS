#pragma once

#undef stdin
#undef stdout
#undef stderr

#include "..\api\api.h"

#include <list>
#include <regex>
#include <vector>

/* Delimiteres in the line */
#define SPACE		'\x20'	
#define PIPE		'\x7C'	/* | */
#define LF			'\x0A'	
#define DASH		'\x2D'	/* - */
#define STROKE		'\x2F'  /* / */
#define DOT			'\x2E'	/* . */
#define COLON		'\x3A'	/* : */

/* Type of handle */
#define WRITE_HANDLE		0
#define READ_HANDLE			1

/* Error messages */
#define ERR_INCORRECT_CMD	"The syntax of the command is incorrect!"
#define ERR_UNKNOWN_CMD		"The command was not found!"
#define ERR_UNXPECTED		"It was unexpected at this time!"

namespace kiv_os_cmd {

	struct cmd_item_t {
		std::string command = "";			/* The command */
		std::string args_line = "";			/* Arguments for the command*/
		bool is_output_redir = false;		/* Redirect to the file - > */
		bool is_input_redir = false;		/* Redirect to the commad - < */
		bool is_append = false;				/* Redirect to the end of file - >> */
		std::string out_dest = "";			/* Name of the file for redirect to the file */
		std::string in_dest = "";			/* Name of the file for redirect to the command */
	};

	class CommandsWrapper {
	private:

		/* Regular expressions for parsing the line */
		static const std::regex r_cmd_line, r_split_pipe, r_command, r_args;
	
		/* Last error message */
		std::string error;

		/* Commands parsed from the line */
		std::list<struct cmd_item_t> commands;

		/**
		 * Parsing line by a delimiter '|'.
		 *
		 * @param cmd_line input line from shell
		 * @return true if has been parsing done well
		 */
		bool Parse_Pipe(const std::string& cmd_line);

		/**
		 * Parse command and arguments from the prepared line by Parse_Pipe.
		 *
		 * @param cmd_item structure represents cmd command with necessary atributes
		 * @return true if has been parsing done well
		 */
		bool Parse_Command(struct cmd_item_t& cmd_item);

		/**
		 * Try to find some redirect in the arguments of the command and put them to the structure.
		 *
		 * @param cmd_item structure represents cmd command with necessary atributes
		 * @return true if has been parsing done well
		 */
		bool Parse_Redirect(struct cmd_item_t& cmd_item);
	
	public:
		CommandsWrapper();

		/**
		 * Start to parse the line from the shell.
		 *
		 * @param line the line from the shell
		 * @return true if has been parsing done well
		 */
		bool Run_Parse(std::string& line);

		/**
		 * Print last error which occurred while parsing the line.
		 */
		void Print_Error();

		/**
		 * Clear all structures to the default values.
		 */
		void Clear();

		/**
		 * Prepare commands for paralel run.
		 *
		 * @return whole handles which were created based on commands
		 */
		std::vector<kiv_os::THandle> Run_Commands();

	}; //class CommandsWrapper

} //namespace kiv_os_cmd