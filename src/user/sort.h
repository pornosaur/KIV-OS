#pragma once
#undef stdin
#undef stdout
#undef stderr
#include "..\api\api.h"

#include <string>
#include <regex>
#include <list>

/* chars which are deleted from input arguments from left and right */
#define ERASE_CHARS " \n\r\t\"'\\"
/* size of read buffer */
#define BUFFER_SIZE 1024

static std::regex reg_sort("[^\\n]+\\n");

/**
* Command SORT
*/
extern "C" size_t __stdcall sort(const kiv_os::TRegisters regs);

/**
* Compare input strings alphabetically ignoring case.
*
* @param first string to compare
* @param second string to compare
* @return true if string first is aphabetically smaller then second.
*/
bool compare_nocase(const std::string& first, const std::string& second);