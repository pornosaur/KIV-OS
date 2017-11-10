#pragma once

#include "..\api\api.h"
#include <vector>
#include <thread>

#define PROC_TABLE_SIZE		1024

struct PCB {
	kiv_os::THandle pid;
	kiv_os::THandle ppid;
	char *proc_name;
	std::vector<kiv_os::THandle> open_files;
	std::thread proc_thread;
};

class ProcFilesystem{
	
public:
	ProcFilesystem();
	~ProcFilesystem();
	std::vector<std::shared_ptr<PCB>> process_table;
	kiv_os::THandle add_process(std::shared_ptr<PCB> pcb);
	std::shared_ptr<PCB> get_process(kiv_os::THandle pid);
	void remove_process(kiv_os::THandle pid);
	void add_open_file(kiv_os::THandle open_file_handle);
	void delete_open_file(kiv_os::THandle open_file_handle);
	std::string pcb_table_to_str();
};