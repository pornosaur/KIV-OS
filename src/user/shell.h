#pragma once

#include "..\api\api.h"
#include "rtl.h"
#include <string>

#define ERROR_RESULT		-1
#define CORRECT_RESULT		1

#define MAX_SIZE_BUFFER_IN	1024
#define REGEX_DEF_GROUP		3

static const char* error_dialog = "The syntax of the command is incorrect.\n";
static const char* cmd_unknown_dialog = "Unknown command...\n";

static const kiv_os::THandle stdinn = kiv_os_rtl::Create_File("CONOUT$", /*FILE_SHARE_WRITE*/2);
static const kiv_os::THandle stdio = kiv_os_rtl::Create_File("CONIN$", /*FILE_SHARE_READ*/1);
static size_t error_write;

extern "C" size_t __stdcall shell(const kiv_os::TRegisters &regs);


//nasledujici funkce si dejte do vlastnich souboru
extern "C" size_t __stdcall type(const kiv_os::TRegisters &regs) { return 0; };
extern "C" size_t __stdcall md(const kiv_os::TRegisters &regs) { return 0; }
extern "C" size_t __stdcall rd(const kiv_os::TRegisters &regs) { return 0; }
extern "C" size_t __stdcall echo(const kiv_os::TRegisters &regs);
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
	{"echo.", echo},	//TODO: promyslet, jestli neudelat tuple?? zalezi, jestli existuje vice variant zapisu i u ostatnich
	{"wc", wc},
	{"sort", sort},
	{"rgen", rgen},
	{"freq", freq},
	{"ps", ps},
	{"shutdown", shutdown}
};

int8_t call_cmd_function(char* cmd_name, const char* params);

/* Convsert c-string to lower case */
void string_to_lower(char* str);

/* Copy std::string to char* */
char *copy_string(const std::string &str);


