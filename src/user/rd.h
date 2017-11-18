#include "../api/api.h"
#include "string.h"
#include "rtl.h"

#include <regex>

#define erase_chars " \n\r\t\"'\\"
#define delimeter "\\"

static std::regex reg_rd("\"([\\S\\s][^\"]+)\"|\'([\\S\\s][^\']+)\'|(\\S+)");
static std::regex reg_rd_params("(/s|/S)|(/q|/Q)|\"([\\S\\s][^\"]+)\"|\'([\\S\\s][^\']+)\'|(\\S+)");

extern "C" size_t __stdcall rd(const kiv_os::TRegisters &regs);

void remove_recursively(std::string &path, const kiv_os::TRegisters &regs);
void remove_subfiles(std::string path, const kiv_os::TRegisters &regs);
void check_params(std::string &parameters, bool &recursively, bool &quiet);