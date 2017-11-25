#include "rgen.h"
#include "rtl.h"
#include "string.h"
size_t __stdcall rgen(const kiv_os::TRegisters regs) {
	size_t written;
	kiv_os::THandle proc_handle;
	std::vector<kiv_os::THandle> proc_handles;
	float random;
	std::string random_str;

	generate = true;

	kiv_os_rtl::Create_Thread((kiv_os::TThread_Proc)wait_for_eof, nullptr, proc_handle);
	while (generate) {
		random = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		random_str = std::to_string(random) + "\n";
		kiv_os_rtl::Write_File(kiv_os::stdOutput, random_str.c_str(), random_str.size(), written);
	}

	proc_handles.push_back(proc_handle);
	kiv_os_rtl::Wait_For(proc_handles, proc_handles.size());
	return 0;
}

void __stdcall wait_for_eof(const void *data) {
	size_t read;
	char *input = (char *)calloc(MAX_SIZE_BUFFER_IN, sizeof(char));

	do {
		kiv_os_rtl::Read_File(kiv_os::stdInput, input, MAX_SIZE_BUFFER_IN, read);
	} while (read != 0);

	free(input);
	generate = false;

}