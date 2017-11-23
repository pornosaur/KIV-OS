#pragma once
#undef stdin
#undef stdout
#undef stderr

#include "../api/api.h"
#include "string.h"

#include <regex>

#define ERASE_CHARS " \n\r\t\"'\\"
/* path delimeter */
#define DELIMETER "\\"
/* size of read buffer */
#define BUFFER_SIZE 1024

extern "C" size_t __stdcall wc(const kiv_os::TRegisters &regs);

inline void calculate(std::string &str, size_t &lines, size_t &words, bool &before_word);