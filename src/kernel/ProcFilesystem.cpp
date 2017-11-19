#include "ProcFilesystem.h"
#include "FileHandler.h"
#include <algorithm>

#include <string>

#include <sstream>
ProcFilesystem::ProcFilesystem() {
	process_table.resize(PROC_TABLE_SIZE, nullptr);
}

ProcFilesystem::~ProcFilesystem() {

}

kiv_os::THandle ProcFilesystem::add_process(std::shared_ptr<PCB> pcb) {
	std::unique_lock<std::mutex> lck(proc_table_mutex);
	std::vector<std::shared_ptr<PCB>>::iterator it = std::find_if(process_table.begin(), process_table.end(),
		[&](std::shared_ptr<PCB> element) { return element == nullptr; });
	if (it != process_table.end()) {
		pcb->pid = static_cast<kiv_os::THandle>(std::distance(process_table.begin(), it));
		process_table[pcb->pid] = pcb;
		return pcb->pid;
	}
	else {
		return kiv_os::erInvalid_Handle;
	}

}

std::shared_ptr<PCB> ProcFilesystem::get_process(kiv_os::THandle pid) {
	std::unique_lock<std::mutex> lck(proc_table_mutex);
	return process_table[pid];	
}

void ProcFilesystem::remove_process(kiv_os::THandle pid) {
	std::unique_lock<std::mutex> lck(proc_table_mutex);
	process_table[pid].reset();
	process_table[pid] = nullptr;
}

int ProcFilesystem::fs_open_object(FileHandler **object, const std::string &absolute_path, unsigned int type) {
	struct dentry *f_dentry = NULL;

	if (absolute_path.compare("procfs") == 0) {
		read_proc = true;
		f_dentry = FS::init_dentry(nullptr, "", 0, 0, 1, 0, 0);
		f_dentry->d_fs = this;
		*object = new FileHandler(0, f_dentry, 0, 1);
	}
	else {
		return ERR_FILE_NOT_FOUND;
	}
	return ERR_SUCCESS;
	
}
int ProcFilesystem::fs_read_file(FileHandler *file, size_t *read_bytes, char *buffer, size_t buffer_size) {
	if (read_proc) {
		std::string pcb_str = pcb_table_to_str();
		if (pcb_str.size() >= buffer_size) {
			pcb_str = pcb_str.substr(0, buffer_size - 1);
		}
		
		*read_bytes = pcb_str.size();
		strcpy_s(buffer, buffer_size, pcb_str.c_str());

	}
	else {
		return ERR_PERMISSION_DENIED;
	}

	return ERR_SUCCESS;
}

std::string ProcFilesystem::pcb_table_to_str() {
	std::unique_lock<std::mutex> lck(proc_table_mutex);
	std::ostringstream oss;
	std::string separator = " | ";
	oss << "pid" << separator << "ppid" << separator << "name" << std::endl;
	for(std::shared_ptr<PCB> pcb : process_table)
	{
		if (pcb != nullptr) {
			oss << pcb->pid << separator << pcb->ppid << separator << pcb->proc_name << std::endl;
		}
		
	}
	
	return oss.str();
}

std::shared_ptr<PCB> ProcFilesystem::get_pcb_by_thread_id(std::thread::id thread_id) {
	std::unique_lock<std::mutex> lck(proc_table_mutex);
	std::vector<std::shared_ptr<PCB>>::iterator it = std::find_if(process_table.begin(), process_table.end(),
		[&](const std::shared_ptr<PCB>& element) {
		if (element == nullptr) { return false; }
		return element->proc_thread.get_id() == thread_id;
	});

	if (it == process_table.end()) {
		return nullptr;
	}

	return *it;
}

std::shared_ptr<PCB> ProcFilesystem::get_pcb_by_handle(kiv_os::THandle handle) {
	std::unique_lock<std::mutex> lck(proc_table_mutex);
	return process_table[handle];
}

void ProcFilesystem::lock_pfs() {
	proc_table_mutex.lock();

}
void ProcFilesystem::unlock_pfs() {
	proc_table_mutex.unlock();

}