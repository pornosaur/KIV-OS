#pragma once

#include "..\api\api.h"
#include "Handler.h"
#include "Handler.h"
#include "FS.h"

#include <vector>
#include <thread>
#include <mutex>
#define PROC_TABLE_SIZE		1024

struct PCB {
	kiv_os::THandle pid;
	kiv_os::THandle ppid;
	char *proc_name;
	std::string workind_dir;
	std::vector<std::shared_ptr<Handler>> open_files;
	std::thread proc_thread;
};

class ProcFilesystem : public FS{
	
public:
	ProcFilesystem();
	~ProcFilesystem();

	int fs_read_file(FileHandler *file, size_t *read_bytes, char *buffer, size_t buffer_size);
	int fs_open_object(FileHandler **object, const std::string &absolute_path, unsigned int type);

	int fs_create_file(FileHandler **file, const std::string &absolute_path) { return ERR_PERMISSION_DENIED; };
	int fs_create_dir(FileHandler **directory, const std::string &absolute_path) { return ERR_PERMISSION_DENIED; };
	int fs_remove_emtpy_dir(FileHandler *file) { return ERR_PERMISSION_DENIED; };
	int fs_read_dir(FileHandler *file, size_t *read_bytes, char *buffer, size_t buffer_size) { return ERR_PERMISSION_DENIED; };
	int fs_write_to_file(FileHandler *file, size_t *writed_bytes, char *buffer, size_t buffer_size) { return ERR_PERMISSION_DENIED; };
	int fs_remove_file(FileHandler *file) { return ERR_PERMISSION_DENIED; };
	int fs_set_file_size(FileHandler *file, size_t file_size) { return ERR_PERMISSION_DENIED; };
	
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
	bool read_proc = false;
};