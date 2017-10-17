#pragma once
#include <regex>

namespace kiv_os_cmd {

	class Arguments {
	private:
		static const std::regex r_cmd_line;

		std::string cmd_line;
		size_t size_line;

	public:
		Arguments(char* line, size_t size);

	}; //class Arguments

} //namespace kiv_os_cmd