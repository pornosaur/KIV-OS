#pragma once
#undef stdin
#undef stdout
#undef stderr
#include "..\api\api.h"
#include "rtl.h"

#include <regex>
#include <string>

#define ERROR_RESULT		-1
#define CORRECT_RESULT		 1

#define REGEX_DEF_GROUP		3

extern "C" size_t __stdcall shell(const kiv_os::TRegisters regs);

/* Function prepared for stop shell */
extern "C" void __stdcall system_stop();

void print_path(char *input, size_t counter, const std::string &str = "");

