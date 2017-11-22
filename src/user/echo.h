#undef stdin
#undef stdout
#undef stderr
#include "../api/api.h"
extern "C" size_t __stdcall echo(const kiv_os::TRegisters &regs);