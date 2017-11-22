#pragma once
#undef stdin
#undef stdout
#undef stderr
#include "../api/api.h"
#include "string.h"

extern "C" size_t __stdcall wc(const kiv_os::TRegisters &regs);

inline void calculate(std::string &str, size_t &lines, size_t &words);