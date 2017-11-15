#pragma once
#include "..\api\api.h"

	extern "C" size_t __stdcall type(const kiv_os::TRegisters &regs);
	extern "C" size_t __stdcall md(const kiv_os::TRegisters &regs);
	extern "C" size_t __stdcall rd(const kiv_os::TRegisters &regs); 
	extern "C" size_t __stdcall echo(const kiv_os::TRegisters &regs); 
	extern "C" size_t __stdcall sort(const kiv_os::TRegisters &regs); 
	extern "C" size_t __stdcall ps(const kiv_os::TRegisters &regs); 
	extern "C" size_t __stdcall shutdown(const kiv_os::TRegisters &regs); 
