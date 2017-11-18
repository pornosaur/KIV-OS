#pragma once

#include "..\api\api.h"

#include "ProcFilesystem.h"

#include "Handler.h"

#include <Windows.h>
#include <vector>
#include <utility>
#include <thread>
#include <mutex>

class ProcessManager {
	public:
		ProcessManager();
		~ProcessManager();
		
		void handle_proc(kiv_os::TRegisters &regs);
		void create_process(char *prog_name, kiv_os::TProcess_Startup_Info *tsi, kiv_os::TRegisters &regs);
		void create_thread(kiv_os::TThread_Proc thread_proc, void *data, kiv_os::TRegisters &regs);
		void wait_for(kiv_os::THandle *proc_handles, size_t proc_count);
		void run_process(kiv_os::TEntry_Point program, kiv_os::TRegisters &regs);
		void ProcessManager::run_thread(kiv_os::TThread_Proc thread_proc, void *data, kiv_os::TRegisters &regs);
		kiv_os::THandle add_open_file(std::shared_ptr<Handler> handle);
		bool close_handle(const kiv_os::THandle handle);
		std::shared_ptr<PCB> get_proc_context();
	private:
		ProcFilesystem *proc_filesystem;
		

		
};
