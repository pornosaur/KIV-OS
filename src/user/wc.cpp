#include "wc.h"
#include "rtl.h"

#include <regex>


size_t __stdcall wc(const kiv_os::TRegisters &regs) {
	kiv_os::TProcess_Startup_Info *tsi = reinterpret_cast<kiv_os::TProcess_Startup_Info*> (regs.rdi.r);
	char *str = tsi->arg;		

	kiv_os::THandle handle = kiv_os::stdInput;		//TODO DELETE this definition, only declaration;
	if (strlen(str) == 0) {
		handle = kiv_os::stdInput;
	}
	else {
		//handle = kiv_os_rtl::Create_File(str, kiv_os::fmOpen_Always);		//TODO open file on disk
	}

	size_t read, lines = 0, words = 0, characters = 0;

	char *input = (char *)malloc(1024 * sizeof(char));
	bool res = true;
	do {
		res = kiv_os_rtl::Read_File(handle, input, 1024, read);
		std::string str_input(input, read);

		calculate(str_input, lines, words);

		characters += read;
	} while (res && read > 0);

	std::string out = "\t" + std::to_string(lines) + " \t" + std::to_string(words) + " \t" + std::to_string(characters) + "\n";	//TODO FILE NAME
	const char *output = kiv_os_str::copy_string(out);

	size_t written;
	kiv_os_rtl::Write_File(kiv_os::stdOutput, output, out.size(), written);

	free(str);
	str = NULL;		//TODO clear this pointer?

	return 0;
}

void calculate(std::string &str, size_t &lines, size_t &words)
{
	std::smatch match;
	static std::regex reg("([^\\s]*)(\\s*)");

	while (!str.empty() && std::regex_search(str, match, reg)) {
		if (!match[2].str().empty()) {
			std::string tmp = match[2].str();
			lines += std::count(tmp.begin(), tmp.end(), '\n');
		}

		words++;
		str = match.suffix();
	}
}