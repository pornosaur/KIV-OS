#pragma once

#include "..\api\api.h"
#include <cassert>
#include <string>

#define path_erase_chr "\\."
#define delimeter "\\"
#define delimeter_size 1

void HandleIO(kiv_os::TRegisters &regs);

void create_file(kiv_os::TRegisters &regs);
void write_file(kiv_os::TRegisters &regs);
void read_file(kiv_os::TRegisters &regs);
void delete_file(kiv_os::TRegisters &regs);
void set_file_position(kiv_os::TRegisters &regs);
void get_file_position(kiv_os::TRegisters &regs);
void close_handle(kiv_os::TRegisters &regs);
void get_current_directory(kiv_os::TRegisters &regs);
void set_current_directory(kiv_os::TRegisters &regs);
void create_pipe(kiv_os::TRegisters &regs);

void set_error(kiv_os::TRegisters &regs, uint16_t code);
void path_compiler(std::string &path);