#pragma once

#include "..\api\api.h"
#include "kernel.h"
#include <Windows.h>
#include "proc_filesystem.h"
void Handle_Proc(kiv_os::TRegisters &regs);
void Create_Process(kiv_os::TRegisters &regs);
void Wait_For(kiv_os::TRegisters &regs);