#pragma once

#include "..\api\api.h"
#include "kernel.h"
#include <Windows.h>
#include <vector>
#include <utility>
#include "proc_filesystem.h"

class ProcessManager {
	public:
		ProcessManager();
		~ProcessManager();
		
		void handle_proc(kiv_os::TRegisters &regs);
		void create_process(char *prog_name, kiv_os::TProcess_Startup_Info *tsi, kiv_os::TRegisters &regs);
		void create_thread(kiv_os::TThread_Proc thread_proc, void *data, kiv_os::TRegisters &regs);
		void wait_for(kiv_os::THandle *proc_handles, size_t proc_count);
	private:
		typedef std::pair<kiv_os::THandle, std::thread> thread_item;
		std::vector<std::pair<kiv_os::THandle, std::thread>> thread_list;
};
