#pragma once

#include <string>

namespace kiv_os_str {

	/* Convert c-string to lower case c-string */
	void string_to_lower(char* str, size_t size);
   
	/* Convert std::string to lower case std::string */
	void string_to_lower(std::string& str);

	/* Copy std::string to char* */
	char *copy_string(const std::string& str);

	/* Get count of char in string */ 
	size_t Get_Count_Char(std::string& str, const char c);

} //namespace kiv_os_str 