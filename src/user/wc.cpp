#include "wc.h"
#include "rtl.h"

#include <vld.h>

static const std::regex reg_wc("\"([\\S\\s][^\"]+)\"|\'([\\S\\s][^\']+)\'|(\\S+)");

size_t __stdcall wc(const kiv_os::TRegisters &regs) {
	kiv_os::TProcess_Startup_Info *tsi = reinterpret_cast<kiv_os::TProcess_Startup_Info*> (regs.rdi.r);
	
	std::smatch match;
	std::string params(tsi->arg);

	params.erase(0, params.find_first_not_of(ERASE_CHARS)); // remove redunadat chars from start of string

	kiv_os::THandle handle;
	bool stdin = false, console = false;

	if (params.empty()) {
		console = stdin = true;
		handle = kiv_os::stdInput;
	}

	char *input = (char *)malloc(BUFFER_SIZE * sizeof(char));
	if (!input) {
		kiv_os_rtl::print_error("Out of memory.");
		return 0;
	}

	size_t total_lines = 0, total_words = 0, total_characters = 0;
	size_t counter = 0;
	while (stdin || (!params.empty() && std::regex_search(params, match, reg_wc))) {
		size_t read, lines = 0, words = 0, characters = 0;

		std::string tmp = match[0].str();
		if (!stdin) {
			params = match.suffix();

			if (!tmp.empty()) {
				tmp.erase(tmp.find_last_not_of(ERASE_CHARS) + 1);
				tmp.erase(0, tmp.find_first_not_of(ERASE_CHARS));

				handle = kiv_os_rtl::Create_File(tmp.c_str(), kiv_os::fmOpen_Always, 0);

				if (!handle) {
					kiv_os_rtl::print_error();
					return 0;
				}

				counter++;
			}
		}

		bool res = false, before_word = false;
		do {
			res = kiv_os_rtl::Read_File(handle, input, BUFFER_SIZE, read);
			std::string str_input(input, read);

			calculate(str_input, lines, words, before_word);

			characters += read;

			if (!stdin) {
				if (!kiv_os_rtl::Close_File(handle)) {
					kiv_os_rtl::print_error();
					return 0;
				}

				read = 0;
			}

		} while (res && read > 0);

		if (before_word) {
			words++;
		}

		total_words += words;
		total_lines += lines;
		total_characters += characters;

		std::string out = "\t" + std::to_string(lines) + " \t" + std::to_string(words)
			+ " \t" + std::to_string(characters) + " " + tmp + "\n";

		size_t written;
		kiv_os_rtl::Write_File(kiv_os::stdOutput, out.c_str(), out.size(), written);

		stdin = false;
	}

	if (!console && counter > 1) {
		std::string out = "\t" + std::to_string(total_lines) + " \t" + std::to_string(total_words)
			+ " \t" + std::to_string(total_characters) + " total\n";

		size_t written;
		kiv_os_rtl::Write_File(kiv_os::stdOutput, out.c_str(), out.size(), written);
	}

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