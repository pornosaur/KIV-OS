#pragma once

#include "..\api\api.h"
#include "BinSemaphore.h"

void Set_Error(const bool failed, kiv_os::TRegisters &regs);

void Lock_Kernel();
void Unlock_Kernel();

extern "C" void __stdcall Sys_Call(kiv_os::TRegisters &regs);
extern "C" void __stdcall Run_VM();
