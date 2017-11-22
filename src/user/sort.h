#pragma once
#include "..\api\api.h"

#include <string>
#include <list>

/* size of read buffer */
#define BUFFER_SIZE 1024

extern "C" size_t __stdcall sort(const kiv_os::TRegisters &regs);

bool compare_nocase(const std::string& first, const std::string& second);