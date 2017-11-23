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

extern "C" size_t __stdcall sort(const kiv_os::TRegisters regs);
static std::regex reg_sort("[^\\n]+");


bool compare_nocase(const std::string& first, const std::string& second);