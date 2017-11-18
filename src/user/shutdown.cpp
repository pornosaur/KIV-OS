#include "shutdown.h"
#include "rtl.h"

size_t __stdcall shutdown(const kiv_os::TRegisters &regs) {
	system_stop();
	return 0;
}
