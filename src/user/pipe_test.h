#pragma once

#include "rtl.h"

#include <cassert>
#include <thread>
#include <mutex>


void pipe_test();

void pipe1(const kiv_os::THandle& writer);
void pipe2(const kiv_os::THandle& reader);
void print_smt(const std::string& str);


