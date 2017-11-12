#pragma once
#include "..\api\api.h"

bool generate = true;
extern "C" size_t __stdcall rgen(const kiv_os::TRegisters &regs);
void wait_for_eof(const void *data);