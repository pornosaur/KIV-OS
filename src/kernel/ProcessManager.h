#pragma once
#undef stdin
#undef stdout
#undef stderr
#include "..\api\api.h"

#include "ProcFilesystem.h"

#include "Handler.h"
#include "console.h"

#include <Windows.h>
#include <vector>
#include <utility>
#include <thread>
#include <mutex>

class ProcessManager {
	public:
		/**
		* Class with actions for processes.
		* @param proc_filesys registered process filesystem
		*/
		ProcessManager(ProcFilesystem *proc_filesys);
		~ProcessManager();
		/**
		* Handle actions of processes from kernel (create process, creat thread, wait for)
		*/
		void handle_proc(kiv_os::TRegisters &regs);
		/*
		* Create process
		*
		* @param prog_name pointer to string which is name of run file
		* @param tsi pointer to TProcess_Startup_Info
		* @param regs structure represent registres Set to rax handle of new process.
		* On error is set flags.carry and in rax is error code.
		*/
		void create_process(char *prog_name, kiv_os::TProcess_Startup_Info *tsi, kiv_os::TRegisters &regs);
		/**
		* Create thread
		* @param thread_proc pointer to structure of TThread_Proc which is function in user
		* @param data  pointer to data which is input parametr for function
		* @param regs structure represent registres Set to rax handle of new process.
		* On error is set flags.carry and in rax is error code.
		*/
		void create_thread(kiv_os::TThread_Proc thread_proc, void *data, kiv_os::TRegisters &regs);
		/**
		* Waiting for processes with hadnles in proc_handles
		* @param proc_handles pointer to array with process handles
		* @param proc_count count of handles in proc_handles
		* @return handle of last join handle
		*/
		kiv_os::THandle wait_for(kiv_os::THandle *proc_handles, size_t proc_count);
		/**
		* Add handle of file, console or pipe to process
		* @param handle handler of file, console or pipe
		* @return numeric handle
		*/
		kiv_os::THandle add_handle(std::shared_ptr<Handler> handle);
		/**
		* Close handle of file, console or pipe in process
		* @param handle handle number of file, console or pipe
		* return true if closing is ok. False if handle was not find.
		*/
		bool close_handle(const kiv_os::THandle handle);
		/**
		* Initialization of prcesses. Run init proces.
		*/
		void init();
		/**
		* Get actual process workspace dir
		* @return path
		*/
		std::string get_proc_work_dir();
		/**
		* Change workspace dir for actual process
		* @param new path
		*/
		void set_proc_work_dir(std::string working_dir);
		/**
		* Get handler of file, console or pipe by handle number.
		* @param handle number
		* @return handler of file, console or pipe
		*/
		std::shared_ptr<Handler> get_handle_object(const kiv_os::THandle hnd);
	private:
		/**
		* registered process filesystem
		*/
		ProcFilesystem *proc_filesystem;
		/**
		* Closing all handles(file etc.) in process
		*/
		void close_handles();
		/**
		* Run shell in init
		*/
		void run_init();
		/**
		* Return actual running process by thread id
		* @return std::shared_ptr<PCB> PCB of running process
		*/
		std::shared_ptr<PCB> get_proc_context();
		/**
		* Run program from user with arguments
		* @param program function for run program in user
		* @param regs  structure represent registres Set to rax handle of new process.
		* @param args pointer to argument of program
		*/
		void run_process(kiv_os::TEntry_Point program, kiv_os::TRegisters &regs, char *args);
		/**
		* Run function from user with data as params
		* @param program function from user
		* @param args pointer to data which is parameter for function
		* @param regs  structure represent registres Set to rax handle of new process.
		* 
		*/
		void run_thread(kiv_os::TThread_Proc thread_proc, void *data, kiv_os::TRegisters &regs);
};
