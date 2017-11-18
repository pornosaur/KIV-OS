#include "../api/api.h"
#include "string.h"
#include "rtl.h"

#include <regex>

#define erase_chars " \n\r\t\"'/"

static std::regex reg_md_multi("\"([\\S\\s][^\"]+)\"|\'([\\S\\s][^\']+)\'|(\\S+)");
static std::regex reg_md_recur("[^\\/]+");

extern "C" size_t __stdcall md(const kiv_os::TRegisters &regs);

void create_directories(std::string &path);