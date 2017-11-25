#include "string.h"
#include <cassert>

void kiv_os_str::string_to_lower(char* str, size_t size)
{
	for (size_t i = 0; i < size; i++) {
		str[i] = tolower(str[i]);
	}
}

void kiv_os_str::string_to_lower(std::string& str)
{
	size_t size = str.size();

	for (size_t i = 0; i < size; i++) {
		str[i] = tolower(str[i]);
	}
}

char *kiv_os_str::copy_string(const std::string& str)
{
	size_t len = str.size();
	char *new_str = (char*)malloc(sizeof(char) * (len + 1u));
	if (!new_str) {
		return nullptr;
	}

	if (strcpy_s(new_str, len + 1u, str.c_str())) {
		free(new_str);
		return nullptr;
	}

	new_str[len] = '\0';

	assert(str.compare(new_str) == 0);

	return new_str;
}


size_t kiv_os_str::Get_Count_Char(std::string& str, const char c)
{
	size_t count = 0;
	for (std::string::iterator it = str.begin(); it != str.end(); ++it) {
		if (*it == c) {
			count++;
		}
	}

	return count;
}
