#pragma once
#undef stdin
#undef stdout
#undef stderr
#include "..\api\api.h"
#include "Handler.h"
#include "Handler.h"
#include "FS.h"

#include <vector>
#include <thread>
#include <mutex>

#define THREAD_TABLE_SIZE		1024

struct PCB {
	kiv_os::THandle pid;								/* pid of process*/
	kiv_os::THandle ppid;								/* pid of parent process*/
	char *proc_name;									/* process name */
	std::string workind_dir;							/* process working dir*/
	std::vector<std::shared_ptr<Handler>> open_files;	/* open files (handlers) in process */
};


struct TCB {
	kiv_os::THandle tid; /* tid of thread*/
	std::thread proc_thread; /* running thread */
	std::shared_ptr<PCB> pcb; /* pointer to pcb of relevant process*/
};
class ProcFilesystem : public FS{
	
public:
	/**
	* Init process and thread tables
	*/
	ProcFilesystem();
	~ProcFilesystem();
	/**
	* Read from procfilesystem
	* @param file File handler representing read file
	* @param read_bytes return number of read bytes
	* @param buffer for read data
	* @param buffer_size size of buffer
	* @return error codes which are defined in FS.h
	*/
	int fs_read_file(FileHandler *file, size_t *read_bytes, char *buffer, size_t buffer_size);
	/**
	* Open proc if absolute_path is same name
	* @param file File handler representing created file
	* @param absolute_path of created directory
	* @return error codes which are defined in api.h
	*/
	int fs_open_object(FileHandler **object, const std::string &absolute_path, unsigned int type);
	/**
	* Not implemented
	* @return ERR_PERMISSION_DENIED
	*/
	int fs_create_file(FileHandler **file, const std::string &absolute_path) { return ERR_PERMISSION_DENIED; };
	/**
	* Not implemented
	* @return ERR_PERMISSION_DENIED
	*/
	int fs_create_dir(FileHandler **directory, const std::string &absolute_path) { return ERR_PERMISSION_DENIED; };
	/**
	* Not implemented
	* @return ERR_PERMISSION_DENIED
	*/
	int fs_remove_emtpy_dir(FileHandler *file) { return ERR_PERMISSION_DENIED; };
	/**
	* Not implemented
	* @return ERR_PERMISSION_DENIED
	*/
	int fs_read_dir(FileHandler *file, size_t *read_bytes, char *buffer, size_t buffer_size) { return ERR_PERMISSION_DENIED; };
	/**
	* Not implemented
	* @return ERR_PERMISSION_DENIED
	*/
	int fs_write_to_file(FileHandler *file, size_t *writed_bytes, char *buffer, size_t buffer_size) { return ERR_PERMISSION_DENIED; };
	/**
	* Not implemented
	* @return ERR_PERMISSION_DENIED
	*/
	int fs_remove_file(FileHandler *file) { return ERR_PERMISSION_DENIED; };
	/**
	* Not implemented
	* @return ERR_PERMISSION_DENIED
	*/
	int fs_set_file_size(FileHandler *file, size_t file_size) { return ERR_PERMISSION_DENIED; };
	/**
	* Add process(pcb) to process_table
	* @param pcb pcb of process
	*/
	void add_process(std::shared_ptr<PCB> pcb);
	/**
	* Add thread to tread_table with pointer to pcb
	* @param pcb pcb of process
	* @return tcb of added thread
	*/
	std::shared_ptr<TCB> add_thread(std::shared_ptr<PCB> pcb);
	/**
	* Remove thread from thread_table by handle
	* @handle handle of deleted thread
	*/
	void remove_thread(kiv_os::THandle handle);
	/**
	* Create string from pcb table.
	*/
	std::string pcb_table_to_str();
	/**
	* lock process filesystem
	*/
	void lock_pfs();
	/**
	* unlock process filesystem
	*/
	void unlock_pfs();
	/**
	* Get pcb by thread id
	* @return return pointer to pcb. If thread_id does not belong to any process return nullptr.
	*/
	std::shared_ptr<PCB> get_pcb_by_thread_id(std::thread::id thread_id);
	/**
	* Get tcb by handle of process
	* @return return pointer to tcb. If handle does not belong to any process return nullptr.
	*/
	std::shared_ptr<TCB> get_tcb_by_handle(kiv_os::THandle handle);

private:
	/**
	* Table of processes
	*/
	std::vector<std::shared_ptr<PCB>> process_table;
	/**
	* Table of threads
	*/
	std::vector<std::shared_ptr<TCB>> thread_table;
	/**
	* mutex for process filesystem
	*/
	std::mutex proc_table_mutex;
	/**
	* flag for reading from proc filesystem
	*/
	bool read_proc = false;
};