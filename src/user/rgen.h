#pragma once
#undef stdin
#undef stdout
#undef stderr
#include "..\api\api.h"

bool generate = true;
extern "C" size_t __stdcall rgen(const kiv_os::TRegisters regs);
void __stdcall wait_for_eof(const void *data);