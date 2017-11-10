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
	return process_table[pid];	
}

void ProcFilesystem::remove_process(kiv_os::THandle pid) {
	process_table[pid] = nullptr; //is it all right?

}


void ProcFilesystem::add_open_file(kiv_os::THandle open_file_handle) {
	//TODO
}
void ProcFilesystem::delete_open_file(kiv_os::THandle open_file_handle) {
	//TODO
}
std::string ProcFilesystem::pcb_table_to_str() {
	std::ostringstream oss;
	std::string separator = " | ";
	for each (std::shared_ptr<PCB> pcb in process_table)
	{
		if (pcb != nullptr) {
			oss << pcb->pid << separator << pcb->ppid << separator << pcb->proc_name << std::endl;
		}
		
	}
	
	return oss.str();
}