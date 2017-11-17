#pragma once

#include "..\api\api.h"
#include "rtl.h"
#include <string>

#define ERROR_RESULT		-1
#define CORRECT_RESULT		 1

#define REGEX_DEF_GROUP		3


/* Variable for loop in the shell */
//int run_shell = 1;

extern "C" size_t __stdcall shell(const kiv_os::TRegisters &regs);

/* Function prepared for stop shell */
extern "C" void __stdcall shell_stop();

