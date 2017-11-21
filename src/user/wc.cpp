#include "wc.h"
#include "rtl.h"

#include <regex>
#include <vld.h>


size_t __stdcall wc(const kiv_os::TRegisters &regs) {
	kiv_os::TProcess_Startup_Info *tsi = reinterpret_cast<kiv_os::TProcess_Startup_Info*> (regs.rdi.r);
	
	char *str = tsi->arg;		

	kiv_os::THandle handle = tsi->stdin_t;;
	/*if (strlen(str) == 0) {
		handle = kiv_os::stdInput;
	}
	else {
		handle = kiv_os_rtl::Create_File(str, kiv_os::fmOpen_Always, 0);		//TODO open file on disk
	}*/


	size_t read, lines = 0, words = 0, characters = 0;

	char *input = (char *)malloc(1024 * sizeof(char));

	bool res = false, before_word = false;
	do {
		res = kiv_os_rtl::Read_File(kiv_os::stdInput, input, 1024, read);
		std::string str_input(input, read);

		calculate(str_input, lines, words, before_word);

		characters += read;
	} while (res && read > 0);

	if (before_word) {
		words++;
	}

	std::string out = "\t" + std::to_string(lines) + " \t" + std::to_string(words)
		+ " \t" + std::to_string(characters) + " " + str +"\n";

	size_t written;
	kiv_os_rtl::Write_File(kiv_os::stdOutput, out.c_str(), out.size(), written);

	free(input);
	input = nullptr;

	return 0;
}

void calculate(std::string &str, size_t &lines, size_t &words, bool &before_word/*, size_t &characters*/)
{
	std::smatch match;
	static std::regex reg("([^\\s]*)(\\s*)");

	while (!str.empty() && std::regex_search(str, match, reg)) {
		if ((match[1].str().empty() && before_word) || (!match[2].str().empty() && !match[1].str().empty())) {
			words++;
		}

		if (!match[2].str().empty()) {
			std::string tmp = match[2].str();
			size_t a = std::count(tmp.begin(), tmp.end(), '\n');
			lines += a;
			before_word = false;
		}
		else {
			before_word = true;
		}

		str = match.suffix();
	}
}