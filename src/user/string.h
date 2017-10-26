#pragma once

#include <string>

namespace kiv_os_str {

	/* Consert c-string to lower case c-string */
   void string_to_lower(char* str, size_t size);

   void string_to_lower(std::string& str);

	/* Copy std::string to char* */
	char *copy_string(const std::string& str);

}