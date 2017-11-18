#include "../api/api.h"
#include "string.h"
#include "rtl.h"

#include <regex>

static std::regex reg_md("\"([\\S\\s][^\"]+)\"|\'([\\S\\s][^\']+)\'|(\\S+)");

extern "C" size_t __stdcall md(const kiv_os::TRegisters &regs);