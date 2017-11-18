#include "../api/api.h"
#include "string.h"
#include "rtl.h"

#include <regex>

#define erase_chars " \n\r\t\"'/"
#define buffer_size 1024

static std::regex reg_type("\"([\\S\\s][^\"]+)\"|\'([\\S\\s][^\']+)\'|(\\S+)");

extern "C" size_t __stdcall type(const kiv_os::TRegisters &regs);

void read_and_write(kiv_os::THandle &in, kiv_os::THandle out);
void write_file_name(int &counter, kiv_os::THandle stdout_t, std::string &name);
bool is_string_name_lower(std::string name, std::string string);