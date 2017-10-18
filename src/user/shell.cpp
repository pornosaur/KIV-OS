#include "shell.h"

#include "rtl.h"

#include <cstring>
#include <cstdlib>

size_t __stdcall shell(const kiv_os::TRegisters &regs) {
	kiv_os::THandle stdin = kiv_os_rtl::Create_File("CONOUT$", /*FILE_SHARE_WRITE*/2);	//nahradte systemovym resenim, zatim viz Console u CreateFile na MSDN
	kiv_os::THandle stdio = kiv_os_rtl::Create_File("CONIN$", /*FILE_SHARE_READ*/1);

	const char* hello = "Hello world!\n";
	char *input = (char *)malloc(sizeof(char) * 255);
	size_t written;
	size_t read;
	kiv_os_rtl::Write_File(stdin, hello, /*strlen(hello)*/13, written);
	//kiv_os_rtl::Read_File(stdio, input, 255, read);
	kiv_os_rtl::Read_File(kiv_os::stdInput, input, 255, read);

	kiv_os_rtl::Write_File(stdin, input, read, written);
	kiv_os_rtl::Close_File(stdin);
	return 0;
}

int8_t call_cmd_function(const char* name, const kiv_os::TRegisters &regs)
{
	uint8_t i, size = sizeof(Cmd_Function) / sizeof(Cmd_Function[0]);

	for (i = 0; i < size; i++) {
		if (Cmd_Function[i].cmd_fun && (strcmp(name, Cmd_Function[i].name) == 0)) {
			Cmd_Function[i].cmd_fun(regs);
			return 0;
		}
	}

	return -1;
}