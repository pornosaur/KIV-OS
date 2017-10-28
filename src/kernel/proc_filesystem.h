#pragma once

#include "..\api\api.h"
#include "thread"
struct PCB {
	kiv_os::THandle PID;
	char *proc_name;
	kiv_os::THandle stdin_t;
	kiv_os::THandle stdout_t;
	kiv_os::THandle stderr_t;
	std::thread proc_thread;
};

kiv_os::THandle Add_Process(std::shared_ptr<PCB> pcb);
std::shared_ptr<PCB> Get_Process(kiv_os::THandle pid);
void Remove_Process(kiv_os::THandle pid);