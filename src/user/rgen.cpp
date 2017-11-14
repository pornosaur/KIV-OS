#include "rgen.h"
#include "rtl.h"
#include "string.h"
size_t __stdcall rgen(const kiv_os::TRegisters &regs) {
	size_t written;
	kiv_os::THandle proc_handle;
	std::vector<kiv_os::THandle> proc_handles;
	float random;
	std::string random_str;
	generate = true;

	kiv_os_rtl::Create_Thread((kiv_os::TThread_Proc)wait_for_eof, nullptr, proc_handle);
	while (generate) {
		random = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		random_str = std::to_string(random);
		kiv_os_rtl::Write_File(kiv_os::stdOutput, kiv_os_str::copy_string(random_str), random_str.length(), written);
	}

	proc_handles.push_back(proc_handle);
	kiv_os_rtl::Wait_For(proc_handles, proc_handles.size());
	return 0;
}

void wait_for_eof(const void *data) {
	size_t read;
	char *input = (char *)calloc(1024, sizeof(char)); //TODO MAX_SIZE const

	do {
		kiv_os_rtl::Read_File(kiv_os::stdInput, input, 1024, read); //TODO MAX_SIZE const
	} while (read != 0);
	kiv_os_rtl::Read_File(kiv_os::stdInput, input, 1024, read); //TODO MAX_SIZE const

	free(input);
	generate = false;

}