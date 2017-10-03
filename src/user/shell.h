#pragma once

#include "..\api\api.h"

extern "C" size_t __stdcall shell(const kiv_os::TRegisters &regs);


//nasledujici funkce si dejte do vlastnich souboru
extern "C" size_t __stdcall type(const kiv_os::TRegisters &regs) { return 0; };
extern "C" size_t __stdcall md(const kiv_os::TRegisters &regs) { return 0; }
extern "C" size_t __stdcall rd(const kiv_os::TRegisters &regs) { return 0; }
extern "C" size_t __stdcall echo(const kiv_os::TRegisters &regs) { return 0; }
extern "C" size_t __stdcall wc(const kiv_os::TRegisters &regs) { return 0; }
extern "C" size_t __stdcall sort(const kiv_os::TRegisters &regs) { return 0; }
extern "C" size_t __stdcall rgen(const kiv_os::TRegisters &regs) { return 0; }
extern "C" size_t __stdcall freq(const kiv_os::TRegisters &regs) { return 0; }
extern "C" size_t __stdcall ps(const kiv_os::TRegisters &regs) { return 0; }
extern "C" size_t __stdcall shutdown(const kiv_os::TRegisters &regs) { return 0; }


/* Tabulka muze byt max. velikosti uint8_t => 256 - pro nase ucely postacujici */
const static struct {
	const char* name;
	size_t(__stdcall *cmd_fun)(const kiv_os::TRegisters&) = NULL;
} Cmd_Function [] = {
	{"type", type},
	{"md", md},
	{"rd", rd},
	{"echo", echo},
	{"wc", wc},
	{"sort", sort},
	{"rgen", rgen},
	{"freq", freq},
	{"ps", ps},
	{"shutdown", shutdown}
};

int8_t call_cmd_function(const char* name, const kiv_os::TRegisters &regs);
