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

/* Function prepared for stop shell */
extern "C" void __stdcall shell_stop();
