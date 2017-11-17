#pragma once

#include "..\api\api.h"
#include "Handler.h"
#include <vector>
#include <thread>
#include <mutex>
#define PROC_TABLE_SIZE		1024

struct PCB {
	kiv_os::THandle pid;
	kiv_os::THandle ppid;
	char *proc_name;
	std::vector<std::shared_ptr<Handler>> open_files;
	std::thread proc_thread;
};

class ProcFilesystem{
	
public:
	ProcFilesystem();
	~ProcFilesystem();
	
	kiv_os::THandle add_process(std::shared_ptr<PCB> pcb);
	std::shared_ptr<PCB> get_process(kiv_os::THandle pid);
	void remove_process(kiv_os::THandle pid);
	std::string pcb_table_to_str();
	void lock_pfs();
	void unlock_pfs();
	std::shared_ptr<PCB> get_pcb_by_thread_id(std::thread::id thread_id);
	std::shared_ptr<PCB> get_pcb_by_handle(kiv_os::THandle handle);

private:
	std::vector<std::shared_ptr<PCB>> process_table;
	std::mutex proc_table_mutex;
};