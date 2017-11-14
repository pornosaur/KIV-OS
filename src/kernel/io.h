#pragma once

#include "..\api\api.h"
#include <cassert>

void HandleIO(kiv_os::TRegisters &regs);
void create_file(kiv_os::TRegisters &regs);
void write_file(kiv_os::TRegisters &regs);
void read_file(kiv_os::TRegisters &regs);
void close_handle(kiv_os::TRegisters &regs);
void create_pipe(kiv_os::TRegisters &regs);
void set_file_position(kiv_os::TRegisters &regs);
void get_file_position(kiv_os::TRegisters &regs);