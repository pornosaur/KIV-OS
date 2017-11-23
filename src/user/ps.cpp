#include "ps.h"
#include "rtl.h"

size_t __stdcall ps(const kiv_os::TRegisters regs) {
	size_t read, written;
	char *input = (char *)calloc(MAX_SIZE_BUFFER_IN, sizeof(char));

	kiv_os::THandle handle = kiv_os_rtl::Create_File("0:\\procfs", kiv_os::fmOpen_Always, kiv_os::faRead_Only);
	bool result = kiv_os_rtl::Read_File(handle, input, MAX_SIZE_BUFFER_IN, read);
	kiv_os_rtl::Write_File(kiv_os::stdOutput, input, read, written);
	kiv_os_rtl::Close_File(handle);

	free(input);
	return 0;
}
