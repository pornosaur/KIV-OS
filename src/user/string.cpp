#include "string.h"

void kiv_os_str::string_to_lower(char* str, size_t size)
{
	for (size_t i = 0; i < size; i++) {
		str[i] = tolower(str[i]);
	}
}

char *kiv_os_str::copy_string(const std::string& str)
{
	size_t len = str.size();
	char *new_str = (char*)malloc(sizeof(char) * (len + 1u));
	strcpy_s(new_str, len + 1u, str.c_str());
	new_str[len] = '\0';

	return new_str;
}