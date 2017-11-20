#include "../api/api.h"
#include "string.h"
#include "rtl.h"

#include <regex>

#define erase_chars " \n\r\t\"'\\"
#define delimeter "\\"
#define buffer_size 1024

static std::regex reg_dir("(/s|/S)|\"([\\S\\s][^\"]+)\"|\'([\\S\\s][^\']+)\'|(\\S+)");

extern "C" size_t __stdcall dir(const kiv_os::TRegisters &regs);

bool print_directory(std::string path, const kiv_os::TRegisters &regs, bool &recursively);
bool print_entry(kiv_os::TDir_Entry *entry);
bool print_name(std::string name);
bool get_current_dir(std::string &path);
void check_params(std::string &parameters, bool &recursively);
void dir_print_help();