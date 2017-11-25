#include "freq.h"
#include "rtl.h"

#include <cstring>


size_t __stdcall freq(const kiv_os::TRegisters regs) {
	char *read_buff = (char *)malloc(1024 * sizeof(char));
	if (!read_buff) {
		kiv_os_rtl::print_error("Out of memory.");
		return 0;
	}

	int freq_table[256] = { 0 };
	size_t read = 0;
	bool res_read = false;

	do {
		res_read = kiv_os_rtl::Read_File(kiv_os::stdInput, read_buff, 1024, read);

		if (res_read && read) {
			for (size_t i = 0; i < read; i++) {
				size_t pos = static_cast<size_t>(*(read_buff + i));
				freq_table[pos]++;
			}
		}
	} while (res_read && (read > 0));

	const char *format = "0x%hhx : %d \n";
	size_t max_line_length = 30;

	free(read_buff);
	read_buff = (char *)malloc(max_line_length * sizeof(char));

	if (!read_buff) {
		kiv_os_rtl::print_error("Out of memory.");
		return 0;
	}

	std::string result = "";
	for (int i = 0; i < 256; i++) {
		if (freq_table[i]) {
			sprintf_s(read_buff, max_line_length, format, i, freq_table[i]);
			result += read_buff;
		}
	}

	size_t written;
	bool a = kiv_os_rtl::Write_File(kiv_os::stdOutput, result.c_str(), result.size(), written);

	free(read_buff);
	read_buff = NULL;

	return 0;
}