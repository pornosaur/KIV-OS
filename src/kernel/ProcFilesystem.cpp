#include "ProcFilesystem.h"

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
		pcb->pid = std::distance(process_table.begin(), it);
		process_table[pcb->pid] = pcb;
		return pcb->pid;
	}
	else {
		//TODO vratit chybovy kod
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

std::string ProcFilesystem::pcb_table_to_str() {
	std::unique_lock<std::mutex> lck(proc_table_mutex);
	std::ostringstream oss;
	std::string separator = " | ";
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