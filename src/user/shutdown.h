#pragma once
#undef stdin
#undef stdout
#undef stderr
#include "../api/api.h"
#include "shell.h"
extern "C" size_t __stdcall shutdown(const kiv_os::TRegisters &regs);